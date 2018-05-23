#pragma once
#include "stdafx.h"
#include <stdio.h>

#include "mscoree.h"
#include <wchar.h>
#include<string>
#include <windows.h>
#include "palclr.h"
class CoreHost
{
public:
    CoreHost(std::wstring _appPath, std::wstring _hostName)
    {
        size_t outSize;
        if (_wgetenv_s(&outSize, coreRoot, MAX_LONGPATH, W("CORE_ROOT")) != 0 && outSize <= 0)
        {
            //TODO 邮件提醒
            return;
        }

        std::wstring coreRootTmp = coreRoot;
        coreRootTmp.append(L"\\").append(coreCLRDll);
        HMODULE coreCLRModule = loadCoreClr(coreRootTmp);
        getRuntimeHost(coreCLRModule);
        setRuntimeHost();
        domainInit(_appPath, _hostName);
    }

    template<typename T>
    T* GetFunctionPointer(std::wstring dllName, std::wstring targetType, std::wstring funcName)
    {
        void* pfnDelegate = NULL;
        HRESULT hr = runtimeHost->CreateDelegate(
            domainId,
            dllName.c_str(),              // Target managed assembly
            targetType.c_str(),              // Target managed type
            funcName.c_str(),                            // Target entry point (static method)
            (INT_PTR*)&pfnDelegate);
        if (hr != S_OK)
        {
            return nullptr;
        }
        return (T*)pfnDelegate;
    }
    ~CoreHost()
    {
        runtimeHost->UnloadAppDomain(domainId, true /* Wait until unload complete */);
        runtimeHost->Stop();
        runtimeHost->Release();
    }
private:
    HMODULE loadCoreClr(std::wstring clrPath)
    {
        return LoadLibraryExW(clrPath.c_str(), NULL, 0);
    };
    void getRuntimeHost(HMODULE clrModule)
    {
        FnGetCLRRuntimeHost pfnGetCLRRuntimeHost =
            (FnGetCLRRuntimeHost)::GetProcAddress(clrModule, "GetCLRRuntimeHost");
        if (!pfnGetCLRRuntimeHost)
        {
            //TODO 邮件提醒
            return;
        }
        HRESULT hr = pfnGetCLRRuntimeHost(IID_ICLRRuntimeHost2, (IUnknown**)&runtimeHost);
    }
    void setRuntimeHost()
    {
        HRESULT hr = runtimeHost->SetStartupFlags(
            // These startup flags control runtime-wide behaviors.
            // A complete list of STARTUP_FLAGS can be found in mscoree.h,
            // but some of the more common ones are listed below.
            static_cast<STARTUP_FLAGS>(
                // STARTUP_FLAGS::STARTUP_SERVER_GC |								// Use server GC
                // STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN |		// Maximize domain-neutral loading
                // STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST |	// Domain-neutral loading for strongly-named assemblies
                STARTUP_FLAGS::STARTUP_CONCURRENT_GC |						// Use concurrent GC
                STARTUP_FLAGS::STARTUP_SINGLE_APPDOMAIN |					// All code executes in the default AppDomain 
                                                                            // (required to use the runtimeHost->ExecuteAssembly helper function)
                STARTUP_FLAGS::STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN	// Prevents domain-neutral loading
                )
        );
        hr = runtimeHost->Start();
    }

    void domainInit(std::wstring appPath, std::wstring hostName)
    {
        trustedPlatformAssemblies[0] = L'\0';

        for (int i = 0; i < _countof(tpaExtensions); i++)
        {
            // Construct the file name search pattern
            std::wstring searchPath = coreRoot;
            searchPath.append(L"\\").append(tpaExtensions[i]);

            // Find files matching the search pattern
            WIN32_FIND_DATAW findData;
            HANDLE fileHandle = FindFirstFileW(searchPath.c_str(), &findData);

            if (fileHandle != INVALID_HANDLE_VALUE)
            {
                do
                {
                    // Construct the full path of the trusted assembly
                    std::wstring pathToAdd;
                    pathToAdd = coreRoot;
                    pathToAdd.append(L"\\").append(findData.cFileName);

                    // Check to see if TPA list needs expanded
                    //if (wcslen(pathToAdd) + (3) + wcslen(trustedPlatformAssemblies) >= tpaSize)
                    //{
                    //	// Expand, if needed
                    //	tpaSize *= 2;
                    //	wchar_t* newTPAList = new wchar_t[tpaSize];
                    //	wcscpy_s(newTPAList, tpaSize, trustedPlatformAssemblies);
                    //	trustedPlatformAssemblies = newTPAList;
                    //}

                    // Add the assembly to the list and delimited with a semi-colon
                    trustedPlatformAssemblies.append(pathToAdd).append(L";");

                    // Note that the CLR does not guarantee which assembly will be loaded if an assembly
                    // is in the TPA list multiple times (perhaps from different paths or perhaps with different NI/NI.dll
                    // extensions. Therefore, a real host should probably add items to the list in priority order and only
                    // add a file if it's not already present on the list.
                    //
                    // For this simple sample, though, and because we're only loading TPA assemblies from a single path,
                    // we can ignore that complication.
                } while (FindNextFileW(fileHandle, &findData));
                FindClose(fileHandle);
            }

            ;
            const  wchar_t* targetAppPath = appPath.c_str();
            // Just use the targetApp provided by the user and remove the file name
            appPaths = targetAppPath;

            // APP_NI_PATHS
            // App (NI) paths are the paths that will be probed for native images not found on the TPA list. 
            // It will typically be similar to the app paths.
            // For this sample, we probe next to the app and in a hypothetical directory of the same name with 'NI' suffixed to the end.

            appNiPaths = targetAppPath;
            appNiPaths.append(L";").append(targetAppPath).append(L"NI");

            // NATIVE_DLL_SEARCH_DIRECTORIES
            // Native dll search directories are paths that the runtime will probe for native DLLs called via PInvoke

            nativeDllSearchDirectories = targetAppPath;
            nativeDllSearchDirectories.append(L";").append(coreRoot);


            // PLATFORM_RESOURCE_ROOTS
            // Platform resource roots are paths to probe in for resource assemblies (in culture-specific sub-directories)

            platformResourceRoots = targetAppPath;


            const wchar_t* propertyValues[] = {
                trustedPlatformAssemblies.c_str(),
                appPaths.c_str(),
                appNiPaths.c_str(),
                nativeDllSearchDirectories.c_str(),
                platformResourceRoots.c_str(),
                appDomainCompatSwitch.c_str()
            };

            HRESULT hr = runtimeHost->CreateAppDomainWithManager(
                hostName.c_str(),		// Friendly AD name
                appDomainFlags,
                NULL,							// Optional AppDomain manager assembly name
                NULL,							// Optional AppDomain manager type (including namespace)
                sizeof(propertyKeys) / sizeof(wchar_t*),
                propertyKeys,
                propertyValues,
                &domainId);
        }
    }
private:
    DWORD domainId;
    wchar_t coreRoot[MAX_LONGPATH];
    ICLRRuntimeHost2* runtimeHost;
    const wchar_t *coreCLRDll = W("CoreCLR.dll");
    int appDomainFlags =
        // APPDOMAIN_FORCE_TRIVIAL_WAIT_OPERATIONS |		// Do not pump messages during wait
        // APPDOMAIN_SECURITY_SANDBOXED |					// Causes assemblies not from the TPA list to be loaded as partially trusted
        APPDOMAIN_ENABLE_PLATFORM_SPECIFIC_APPS |			// Enable platform-specific assemblies to run
        APPDOMAIN_ENABLE_PINVOKE_AND_CLASSIC_COMINTEROP |	// Allow PInvoking from non-TPA assemblies
        APPDOMAIN_DISABLE_TRANSPARENCY_ENFORCEMENT;			// Entirely disables transparency checks 
                                                            // TRUSTED_PLATFORM_ASSEMBLIES
                                                            // "Trusted Platform Assemblies" are prioritized by the loader and always loaded with full trust.
                                                            // A common pattern is to include any assemblies next to CoreCLR.dll as platform assemblies.
                                                            // More sophisticated hosts may also include their own Framework extensions (such as AppDomain managers)
                                                            // in this list.

    const wchar_t* tpaExtensions[3] = {
        L"*.dll",
        L"*.exe",
        L"*.winmd"
    };

    //Setup key pairs for AppDomain  properties
    const wchar_t* propertyKeys[6] = {
        L"TRUSTED_PLATFORM_ASSEMBLIES",
        L"APP_PATHS",
        L"APP_NI_PATHS",
        L"NATIVE_DLL_SEARCH_DIRECTORIES",
        L"PLATFORM_RESOURCE_ROOTS",
        L"AppDomainCompatSwitch"
    };
    //value pairs for AppDomain  properties
    std::wstring trustedPlatformAssemblies;  // Starting size for our TPA (Trusted Platform Assemblies) list
    std::wstring appPaths;
    std::wstring appNiPaths;
    std::wstring nativeDllSearchDirectories;
    std::wstring platformResourceRoots;
    // Typically the latest behavior is desired, but some hosts may want to default to older Silverlight
    // or Windows Phone behaviors for compatibility reasons.
    std::wstring appDomainCompatSwitch = L"UseLatestBehaviorWhenTFMNotSpecified";
};
