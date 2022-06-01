using System.Diagnostics;
using UnityEditor.WebGL.Emscripten;
using UnityEditor.Utils;
using Debug = UnityEngine.Debug;

namespace UnityEditor.WebGL
{
    // This is a ScriptableSingleton, so it gets recreated on script reload.
    internal class WebsockifyEditorWrapper : ScriptableSingleton<WebsockifyEditorWrapper>
    {
        static Process process;

        WebsockifyEditorWrapper()
        {
            if (process == null)
            {
                var websockifyPath = EmscriptenPaths.buildToolsDir + "/websockify";
                var processStartInfo = new ProcessStartInfo(EmscriptenPaths.nodeExecutable)
                {
                    // ports must match PLAYER_DIRECTCONNECT_PORT_WEBSOCKET and PLAYER_DIRECTCONNECT_PORT and GeneralConnection.h
                    Arguments = "\"" + websockifyPath + "/websockify.js\" 54998 localhost:" + UnityEditorInternal.ProfilerDriver.directConnectionPort,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                };

                processStartInfo.EnvironmentVariables["NODE_PATH"] = websockifyPath + "/node_modules";

                process = new Process();
                process.StartInfo = processStartInfo;
                process.Start();

                // Make sure we can stop output reading to avoid deadlocks.
                System.AppDomain.CurrentDomain.DomainUnload += OnUnload;

                // We need to read the output streams, because node will fail on windows when the streams are not read.
                // ("Uncaught Error: Implement me. Unknown stream file type! "), known issue.
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                if (process == null)
                    Debug.LogError("Could not start WebSocket bridge.");
            }
        }

        static void OnUnload(object sender, System.EventArgs e)
        {
            if (process != null)
            {
                // Make sure the process is dead, then stop the threads reading output - otherwise, we will get a deadlock when unloading the domain.
                if (!process.HasExited)
                    process.Kill();
                process.CancelErrorRead();
                process.CancelOutputRead();
                process = null;
            }
        }

        public static void CreateIfNeeded()
        {
            // Referencing instance should create the wrapper object.
            if (instance == null)
                Debug.LogError("No Websockify wrapper created");
        }
    }
}
