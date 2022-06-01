using System;
using System.Diagnostics;
using System.Text;
using UnityEditor.Utils;
using UnityEditor.WebGL.Emscripten;
using UnityEngine;

namespace UnityEditor.WebGL
{
    static internal class ProgramUtils
    {
        internal static bool StartProgramChecked(ProcessStartInfo p)
        {
            Console.WriteLine("Filename: " + p.FileName);
            Console.WriteLine("Arguments: " + p.Arguments);

            int responsefileindex = p.Arguments.IndexOf("Temp/UnityTempFile");
            Console.WriteLine("index: " + responsefileindex);

            if (responsefileindex > 0)
            {
                var responsefile = p.Arguments.Substring(responsefileindex);
                Console.WriteLine("Responsefile: " + responsefile + " Contents: ");
                Console.WriteLine(System.IO.File.ReadAllText(responsefile));
            }

            var stdout = new StringBuilder();
            var stderr = new StringBuilder();

            p.RedirectStandardOutput = true;
            p.RedirectStandardError = true;
            p.UseShellExecute = false;

            using (var process = Process.Start(p))
            {
                process.OutputDataReceived += (sender, e) => { stdout.Append(e.Data); };
                process.ErrorDataReceived += (sender, e) => { stderr.Append(e.Data); };

                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                process.WaitForExit();

                if (process.ExitCode != 0)
                {
                    UnityEngine.Debug.LogError("Failed running " + p.FileName + " " + p.Arguments + "\n\nstdout:" + stdout + "\nstderr:" + stderr);

                    throw new Exception("Failed building WebGL Player.");
                }
            }
            return true;
        }
    }
}
