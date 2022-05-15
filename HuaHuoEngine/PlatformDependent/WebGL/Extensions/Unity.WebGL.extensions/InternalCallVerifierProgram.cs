#if ENABLE_WEBGL_ASSEMBLY_VALIDATION
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using UnityEditor.Scripting.Compilers;
using UnityEditor.Utils;
using UnityEngine;

namespace UnityEditor.WebGL
{
    internal class InternalCallVerifierProgram : IDisposable
    {
        private Process m_Process;

        public delegate void OutputDataReceivedHandler(object sendingProcess, DataReceivedEventArgs outLine);
        public delegate void ErrorDataReceivedHandler(object sendingProcess, DataReceivedEventArgs outLine);

        public event OutputDataReceivedHandler OutputDataReceived;
        public event ErrorDataReceivedHandler ErrorDataReceived;

        public static InternalCallVerifierProgram Managed(string[] userAssemblies)
        {
            return new InternalCallVerifierProgram(new ProcessStartInfo
            {
                Arguments = CommandLineFormatter.PrepareFileName(internalCallVerifierPath) + " " + ArgsFrom(userAssemblies),
                CreateNoWindow = true,
                FileName = monoEXEPath,
                RedirectStandardInput = true,
                RedirectStandardError = true,
                RedirectStandardOutput = true,
                WorkingDirectory = Path.GetFullPath(Path.Combine(Application.dataPath, "..")),
                UseShellExecute = false,
                EnvironmentVariables =
                {
                    {"MONO_PATH", MonoBleedingEdgePath("lib", "mono", "4.0")},
                    {"MONO_CFG_DIR", MonoBleedingEdgePath("etc")}
                }
            });
        }

        public static InternalCallVerifierProgram Native(string[] userAssemblies)
        {
            return new InternalCallVerifierProgram(new ProcessStartInfo
            {
                Arguments = ArgsFrom(userAssemblies),
                CreateNoWindow = true,
                FileName = internalCallVerifierPath,
                RedirectStandardInput = true,
                RedirectStandardError = true,
                RedirectStandardOutput = true,
                WorkingDirectory = Path.GetFullPath(Path.Combine(Application.dataPath, "..")),
                UseShellExecute = false
            });
        }

        private static string ArgsFrom(IEnumerable<string> userAssemblies)
        {
            var args = new List<string>
            {
                @"--call-graph=" + CommandLineFormatter.PrepareFileName(graphFilePath),
                @"--mono-lib-path=" + CommandLineFormatter.PrepareFileName(monoLibPath),
                @"--unity-engine-path=" + CommandLineFormatter.PrepareFileName(unityEnginePath),
                "--skip-partially-supported"
            };

            args.AddRange(userAssemblies);
            return args.Aggregate((a, b) => a + " " + b);
        }

        private InternalCallVerifierProgram(ProcessStartInfo info)
        {
            m_Process = new Process {StartInfo = info};
        }

        public int exitCode
        {
            get { return processSafe.ExitCode; }
        }

        public int id
        {
            get { return processSafe.Id; }
        }

        public bool hasExited
        {
            get
            {
                try
                {
                    return processSafe.HasExited;
                }
                catch (InvalidOperationException)
                {
                    return true;
                }
            }
        }

        private static string monoEXEPath
        {
            get
            {
                var monoexe = MonoBleedingEdgePath("bin", "mono");
                if (Application.platform == RuntimePlatform.WindowsEditor)
                    monoexe = CommandLineFormatter.PrepareFileName(monoexe + ".exe");

                return monoexe;
            }
        }

        private static string internalCallVerifierPath
        {
            get
            {
                return new[]
                {
                    BuildPipeline.GetPlaybackEngineDirectory(BuildTarget.WebGL, BuildOptions.None),
                    "BuildTools",
                    "icall-verifier",
                    "icall-verifier.exe"
                }.Aggregate(Path.Combine);
            }
        }

        private static string unityEnginePath
        {
            get
            {
                return new[]
                {
                    BuildPipeline.GetPlaybackEngineDirectory(BuildTarget.WebGL, BuildOptions.None),
                    "Managed"
                }.Aggregate(Path.Combine);
            }
        }

        private static string graphFilePath
        {
            get
            {
                return new[]
                {
                    BuildPipeline.GetPlaybackEngineDirectory(BuildTarget.WebGL, BuildOptions.None),
                    "BuildTools",
                    "monodistribution.unity.callgraph"
                }.Aggregate(Path.Combine);
            }
        }

        private static string monoLibPath
        {
            get
            {
                return new[]
                {
                    MonoInstallationFinder.GetMonoInstallation("MonoBleedingEdge"),
                    "lib",
                    "mono",
                    "unity"
                }.Aggregate(Path.Combine);
            }
        }

        private static string MonoBleedingEdgePath(params string[] parts)
        {
            var path = new List<string> { MonoInstallationFinder.GetMonoInstallation("MonoBleedingEdge") };
            path.AddRange(parts);
            return path.Aggregate(Path.Combine);
        }

        public void Start()
        {
            var process = processSafe;
            process.OutputDataReceived += delegate(object sender, DataReceivedEventArgs args)
            {
                if (OutputDataReceived != null) OutputDataReceived(sender, args);
            };

            process.ErrorDataReceived += delegate(object sender, DataReceivedEventArgs args)
            {
                if (ErrorDataReceived != null) ErrorDataReceived(sender, args);
            };
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
        }

        public void Dispose()
        {
            var process = processSafe;

            try
            {
                if (!hasExited)
                {
                    process.Kill();
                    process.WaitForExit();
                }

                process.Dispose();
            }
            catch (InvalidOperationException)
            {
            }

            m_Process = null;
        }

        public void WaitForExit()
        {
            processSafe.WaitForExit();
        }

        public bool WaitForExit(int milliseconds)
        {
            return processSafe.WaitForExit(milliseconds);
        }

        private Process processSafe
        {
            get
            {
                if (m_Process == null)
                    throw new ObjectDisposedException("m_Process", "Process already disposed!");

                return m_Process;
            }
        }

        public override string ToString()
        {
            return m_Process != null
                ? string.Format("{0} {1}", m_Process.StartInfo.FileName, m_Process.StartInfo.Arguments)
                : "Disposed process!";
        }
    }
}
#endif
