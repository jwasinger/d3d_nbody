using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

using System.IO.MemoryMappedFiles;

namespace DebugConsole
{
    class Program
    {
        static void testSharedMemory()
        {
            using (var mmf = MemoryMappedFile.OpenExisting("Global\\ConsoleFileMap0"))
            {
                using (var accessor = mmf.CreateViewAccessor(0, 512))
                { 
                    char[] str = ("hello world\n").ToCharArray();

                    for (int i = 0; i < str.Length; i++)
                    {
                        accessor.Write(i, str[i]);
                    }
                }
            }
        }

        static void Main(string[] args)
        {
            testSharedMemory();
        }
    }
}
