using Helper;
using System;
using System.Runtime.InteropServices;

namespace DotNetCall
{
    [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi)]
    public delegate void helloDelegate(IntPtr ths,string str);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate  void  CreateDelegate(ref IntPtr ptr);

    struct MyClass
    {
        public IntPtr hello;
    }
   public class Program
    {
        static void Main(string[] args)
        {
           var dllHandle= NativeHelper.LoadLibrary("DotNetCalled.dll");
           var createAddr = NativeHelper.GetProcAddress(dllHandle, "Create");
           var create=Marshal.GetDelegateForFunctionPointer<CreateDelegate>(createAddr);
             
           var _objptr=IntPtr.Zero;
           create(ref _objptr);
           IntPtr dst =IntPtr.Zero;
           NativeHelper.RtlMoveMemory(ref dst, _objptr, IntPtr.Size);
           var myclass= Marshal.PtrToStructure<MyClass>(dst);
           var func = Marshal.GetDelegateForFunctionPointer<helloDelegate>(myclass.hello);
           func(_objptr, "helloworld");
        }
    }
}
