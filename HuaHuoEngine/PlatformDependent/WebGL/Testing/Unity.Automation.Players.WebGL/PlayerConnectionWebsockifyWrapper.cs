using System;
using System.Diagnostics;
using Unity.CommonTools;

namespace Unity.Automation.Players.WebGL
{
    public class PlayerConnectionWebsockifyWrapper
    {
        Process process;

        public PlayerConnectionWebsockifyWrapper()
        {
            var websockifyPath = Workspace.ResolvePath("build/WebGLSupport/BuildTools/websockify");
            var nodeExecutable = GetNodeExecutable();
            var processStartInfo = new ProcessStartInfo(nodeExecutable)
            {
                // ports must match PLAYER_DIRECTCONNECT_PORT_WEBSOCKET and PLAYER_DIRECTCONNECT_PORT and GeneralConnection.h
                Arguments = "\"" + websockifyPath + "/websockify.js\" 54998 localhost:" + UnityEditorInternal.ProfilerDriver.directConnectionPort,
                UseShellExecute = false,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
            };

            processStartInfo.EnvironmentVariables["NODE_PATH"] = websockifyPath + "/node_modules";

            process = new Process
            {
                StartInfo = processStartInfo
            };

            process.OutputDataReceived += OutputDataReceivedHandler;
            process.ErrorDataReceived  += ErrorDataReceivedHandler;

            process.Start();

            // We need to read the output streams, because node will fail on windows when the streams are not read.
            // ("Uncaught Error: Implement me. Unknown stream file type! "), known issue.
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
        }

        public void Dispose()
        {
            // Make sure the process is dead, then stop the threads reading output - otherwise, we will get a deadlock when unloading the domain.
            if (process.HasExited)
            {
                Console.WriteLine($"[websockify] Process exit code: {process.ExitCode}");
            }
            else
            {
                process.Kill();
                Console.WriteLine("[websockify] Process killed");
            }
            process.CancelErrorRead();
            process.CancelOutputRead();
        }

        private static void OutputDataReceivedHandler(object sendingProcess, DataReceivedEventArgs outLine)
        {
            if (!string.IsNullOrEmpty(outLine.Data))
            {
                Console.WriteLine($"[websockify] {outLine.Data}");
            }
        }

        private static void ErrorDataReceivedHandler(object sendingProcess, DataReceivedEventArgs outLine)
        {
            if (!string.IsNullOrEmpty(outLine.Data))
            {
                Console.WriteLine($"[websockify] [ERROR] {outLine.Data}");
            }
        }

        private static string GetNodeExecutable()
        {
            if (Workspace.IsWindows())
                return Workspace.ResolvePath("External/nodejs/builds/win64/node.exe");
            if (Workspace.IsOSX())
                return Workspace.ResolvePath("External/nodejs/builds/osx/bin/node");
            if (Workspace.IsLinux())
                return Workspace.ResolvePath("External/nodejs/builds/linux64/bin/node");
            throw new NotImplementedException("Unknown editor platform");
        }
    }
}
