using System;
using System.Runtime.InteropServices;

namespace Helper
{
    public class NativeHelper
    {
        [DllImport("kernel32.dll")]
        public static extern IntPtr LoadLibrary(string path);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool FreeLibrary(IntPtr module);

        [DllImport("kernel32.dll")]
        public static extern IntPtr GetProcAddress(IntPtr lib,string funcName);

        [DllImport("WS2_32.DLL")]
        public static extern int WSAStartup(int wVersionRequested, IntPtr lpWsaData);

        [DllImport("WS2_32.DLL")]
        public static extern int WSACleanup();

        [DllImport("kernel32.dll",CharSet =CharSet.Ansi)]
        public static extern void RtlMoveMemory(ref IntPtr dst, IntPtr src, int size);

        [DllImport("kernel32.dll")]
        public static extern void CopyMemory(ref IntPtr dst, IntPtr src, int size);

        public static int MakeLong(int lowPart, int highPart)
        {
            return (int)(((ushort)lowPart) | (uint)(highPart << 16));
        }

        [DllImport("kernel32")]
        private static extern bool HeapFree(IntPtr hHeep, uint dwFlags, IntPtr lpMem);


        [DllImport("kernel32")]
        private static extern bool HeapAlloc(IntPtr hHeep, uint dwFlags, int lpMem);

        [DllImport("kernel32")]
        private static extern IntPtr GetProcessHeap();

        internal static void HEAP_FREE(IntPtr lpMem)
        {
            IntPtr pHeap = GetProcessHeap();
            HeapFree(pHeap, 0, lpMem);
        }

        internal static void HEAP_ALLOC(int size)
        {
            IntPtr pHeap = GetProcessHeap();
            HeapAlloc(pHeap, 0, size);
        }
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct WsaData
    {
        public readonly ushort wVersion;
        public readonly ushort wHighVersion;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256 + 1)]
        public readonly string szDescription;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128 + 1)]
        public readonly string szSystemStatus;

        public readonly ushort iMaxSockets;
        public readonly ushort iMaxUdpDg;
        public readonly IntPtr lpVendorInfo;
        //char FAR* lpVendorInfo;
    }
}
