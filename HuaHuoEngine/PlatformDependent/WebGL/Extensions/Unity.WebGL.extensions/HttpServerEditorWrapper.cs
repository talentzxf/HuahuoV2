using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using UnityEditor.WebGL.Emscripten;
using UnityEditor.Scripting;
using UnityEditor.Utils;
using UnityEngine;

namespace UnityEditor.WebGL
{
    internal class HttpServerEditorWrapper : ScriptableSingleton<HttpServerEditorWrapper>
    {
        [SerializeField] public string path;
        [SerializeField] public int port;
        [SerializeField] public int pid;
        [SerializeField] public string time;

        void Create(string _path, int _port)
        {
            var exe = EmscriptenPaths.buildToolsDir + "/SimpleWebServer.exe";
            var program = new ManagedProgram(MonoInstallationFinder.GetMonoInstallation("MonoBleedingEdge"), null, exe, "\"" + _path + "\" " + _port + " " + Process.GetCurrentProcess().Id, false, null);

            program._process.Start();

            while (true)
            {
                var line = program._process.StandardOutput.ReadLine();
                if (line == null || line.Contains("Starting web server"))
                    break;
            }

            path = _path;
            port = _port;
            pid = program._process.Id;
            time = program._process.StartTime.ToString();
            Save(true);
        }

        public static int GetRandomUnusedPort()
        {
            var listener = new TcpListener(IPAddress.Any, 0);
            listener.Start();
            var port = ((IPEndPoint)listener.LocalEndpoint).Port;
            listener.Stop();
            return port;
        }

        public static void Kill()
        {
            Process p = null;
            try
            {
                p = Process.GetProcessById(instance.pid);
                p.Kill();
            }
            catch
            {
                // if we could not get a process, there is non alive. continue.
            }
        }

        public static void CreateIfNeeded(string path, out int port)
        {
            // See if we can get an existing process with the PID we remembered.
            Process p = null;
            try
            {
                p = Process.GetProcessById(instance.pid);
            }
            catch
            {
                // if we could not get a process, there is non alive. continue.
            }

            // Check if this process is really the one we used (and not another one reusing the PID).
            if (p != null && p.StartTime.ToString() == instance.time)
            {
                if (Application.platform == RuntimePlatform.WindowsEditor)
                {
                    // On Winodws, if the existing process was re-used, then this caused periodic server failures as
                    // the process is destroyed by the time the browser is opened. Always killing the process and
                    // creating a new one solves this problem.
                    try
                    {
                        p.Kill();
                    }
                    catch
                    {
                    }
                }
                else
                {
                    if (instance.path == path)
                    {
                        // We have a server running for this setup, which we can reuse
                        port = instance.port;
                        return;
                    }
                    else
                    {
                        // This server does not match our setup. Kill it, so we can start a new one.
                        p.Kill();
                    }
                }
            }

            port = GetRandomUnusedPort();
            // No existing server we can use. Start a new one.
            instance.Create(path, port);
        }
    }
}
