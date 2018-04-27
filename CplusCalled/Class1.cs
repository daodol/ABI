using System;

namespace CplusCalled
{
    [System.Runtime.InteropServices.StructLayoutAttribute(System.Runtime.InteropServices.LayoutKind.Sequential, CharSet = System.Runtime.InteropServices.CharSet.Ansi)]
    public struct Test
    {

        /// int
        public int a;

        /// char[12]
        [System.Runtime.InteropServices.MarshalAsAttribute(System.Runtime.InteropServices.UnmanagedType.ByValTStr, SizeConst = 12)]
        public string b;
    }
    public class Class1
    {
        public static void HelloWorld(String abc)
        {
            System.Console.WriteLine(abc);
        }

        public static void TestStruct(ref Test t)
        {
            t.a = 2;
            Console.WriteLine("a:{}",t.a);
            Console.WriteLine("b:{}",t.b);
        }

        public static void TestStruct( Test t)
        {
            t.a = 2;
            Console.WriteLine("a:{}", t.a);
            Console.WriteLine("b:{}", t.b);
        }
    }
}
