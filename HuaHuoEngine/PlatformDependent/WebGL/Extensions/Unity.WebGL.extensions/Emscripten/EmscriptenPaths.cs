using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEditor.Utils;

namespace UnityEditor.WebGL.Emscripten
{
    internal class EmscriptenPaths
    {
        private static string s_BuildToolsDir;
        private static string s_EditorToolsDir;
        private static string s_DataPath;

        [DllImport("kernel32.dll", EntryPoint = "GetShortPathName", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern int WindowsGetShortPathName(
            [MarshalAs(UnmanagedType.LPWStr)]
            string lpszLongPath,
            [MarshalAs(UnmanagedType.LPWStr)]
            StringBuilder lpszShortPath,
            int cchBuffer
        );

        public static string GetShortPathName(string path)
        {
            if (Application.platform != RuntimePlatform.WindowsEditor || Encoding.UTF8.GetByteCount(path) == path.Length)
                return path;
            int length = WindowsGetShortPathName(path, null, 0);
            if (length == 0)
                return path;
            StringBuilder shortPath = new StringBuilder(length);
            length = WindowsGetShortPathName(path, shortPath, shortPath.Capacity);
            if (length == 0)
                return path;
            return shortPath.ToString(0, length);
        }

        public static void SetupDataPath()
        {
            s_DataPath = GetShortPathName(Path.GetFullPath(Application.dataPath));
        }

        public static string dataPath
        {
            get
            {
                if (s_DataPath == null)
                    SetupDataPath();
                return s_DataPath;
            }
        }

        public static void SetupBuildToolsDir()
        {
            s_BuildToolsDir = Paths.Combine(BuildPipeline.GetPlaybackEngineDirectory(BuildTarget.WebGL, BuildOptions.None), "BuildTools");
        }

        public static void SetupEditorToolsDir()
        {
            s_EditorToolsDir = Paths.Combine(EditorApplication.applicationContentsPath, "Tools");
        }

        public static string buildToolsDir
        {
            get
            {
                if (s_BuildToolsDir == null)
                    SetupBuildToolsDir();
                return s_BuildToolsDir;
            }
        }

        public static string editorToolsDir
        {
            get
            {
                if (s_EditorToolsDir == null)
                    SetupEditorToolsDir();
                return s_EditorToolsDir;
            }
        }

        public static string emscriptenPlatformSdkDir
        {
            get
            {
                // Developer users may have activated a custom version of Emscripten SDK, in which case EMSDK
                // environment variable locates the root directory of the SDK.
                string existingEmsdk = Environment.GetEnvironmentVariable("EMSDK");
                if (!string.IsNullOrEmpty(existingEmsdk))
                {
                    UnityEngine.Debug.Log("Using existing developer provided Emscripten SDK installation from '" + existingEmsdk + "'");
                    return existingEmsdk;
                }

                if (IsWindows())
                    return Paths.Combine(buildToolsDir, "Emscripten_Win");
                else if (IsLinux() || IsMac())
                    // Linux and Mac share a platform-independent emscripten sdk
                    return Paths.Combine(buildToolsDir, "Emscripten_Mac");
                else
                    throw new Exception("Unknown platform");
            }
        }

        public static bool IsWindows()
        {
            return Environment.OSVersion.Platform == PlatformID.Win32NT || Environment.OSVersion.Platform == PlatformID.Win32S ||
                Environment.OSVersion.Platform == PlatformID.Win32Windows || Environment.OSVersion.Platform == PlatformID.WinCE;
        }

        public static bool IsLinux()
        {
            return Directory.Exists("/proc");
        }

        public static bool IsMac()
        {
            return !IsLinux() && !IsWindows();
        }

        public static string emscriptenDir
        {
            get
            {
                // Developer users may have activated a custom version of Emscripten SDK, in which case EMSCRIPTEN
                // environment variable locates the root directory of the Emscripten toolchain (emcc.bat etc)
                string existingEmscripten = Environment.GetEnvironmentVariable("EMSCRIPTEN");
                if (!string.IsNullOrEmpty(existingEmscripten))
                    return existingEmscripten;

                return Paths.Combine(buildToolsDir, "Emscripten");
            }
        }

        public static string emlink
        {
            get { return Paths.Combine(emscriptenDir , "emlink.py"); }
        }

        public static string emcc
        {
            get { return Paths.Combine(emscriptenDir, "emcc"); }
        }

        public static string packager
        {
            get { return Paths.Combine(emscriptenDir, "tools", "file_packager.py"); }
        }

        public static string optimizer
        {
            get
            {
                // Developer users may have activated a custom version of Emscripten SDK, in which case EMSCRIPTEN_NATIVE_OPTIMIZER
                // environment variable locates the optimizer.exe executable file
                string existingNativeOptimizer = Environment.GetEnvironmentVariable("EMSCRIPTEN_NATIVE_OPTIMIZER");
                if (!string.IsNullOrEmpty(existingNativeOptimizer))
                    return existingNativeOptimizer;

                return Paths.Combine(llvmDir, "optimizer.exe");
            }
        }

        public static string binaryen
        {
            get
            {
                // Developer users may have activated a custom version of Emscripten SDK, in which case BINARYEN_ROOT
                // environment variable locates the root directory of where Binaryen toolchain is built
                string existingBinaryen = Environment.GetEnvironmentVariable("BINARYEN_ROOT");
                if (!string.IsNullOrEmpty(existingBinaryen))
                    return existingBinaryen;

                return Paths.Combine(llvmDir, "binaryen");
            }
        }

        public static string nodeExecutable
        {
            get
            {
                // Developer users may have activated a custom version of Emscripten SDK, in which case EMSDK_NODE
                // environment variable locates the Node.js executable bundled with Emsdk.
                string existingNode = Environment.GetEnvironmentVariable("EMSDK_NODE");
                if (!string.IsNullOrEmpty(existingNode))
                    return existingNode;

                if (IsWindows())
                    return Paths.Combine(editorToolsDir, "nodejs", "node.exe");
                else if (IsLinux() || IsMac())
                    return Paths.Combine(editorToolsDir, "nodejs", "bin", "node");
                else
                    throw new Exception("Unknown platform");
            }
        }

        static string s_PythonVersion;

        public static string pythonExecutable
        {
            get
            {
                // Developer users may have activated a custom version of Emscripten SDK, in which case EMSDK_PYTHON
                // environment variable locates the Python executable provided with Emsdk.
                string existingPython = Environment.GetEnvironmentVariable("EMSDK_PYTHON");
                if (!string.IsNullOrEmpty(existingPython))
                    return existingPython;

                if (IsWindows())
                    return "\"" + Paths.Combine(emscriptenPlatformSdkDir, "python", "2.7.5.3_64bit", "python.exe") + "\"";

                // Brotli compression tool and Emscripten require Python 2.x. In some situations,
                // such as recent MacOS versions, `python` defaults to Python3. In these cases,
                // python2 command is available to explicitely invoke Python 2.x.
                // Try to use python2 command if it's available, ensuring the use Python 2.x.
                // If not, fall back to the regular python command.
                if (s_PythonVersion == null)
                {
                    ProcessStartInfo startInfo = new ProcessStartInfo("python2")
                    {
                        Arguments = "--version",
                        UseShellExecute = false,
                        CreateNoWindow = true,
                    };

                    try
                    {
                        // Check to see if python2 is successfully executed
                        using (var process = Process.Start(startInfo))
                        {
                            process.WaitForExit();

                            if (process.ExitCode == 0)
                                s_PythonVersion = "python2";
                            else
                                s_PythonVersion = "python";
                        }
                    }
                    catch
                    {
                        s_PythonVersion = "python";
                    }
                }

                return s_PythonVersion;
            }
        }

        public static string nmExecutable
        {
            get
            {
                var nm = Paths.Combine(llvmDir, "llvm-nm");
                if (IsWindows())
                    nm += ".exe";
                return nm;
            }
        }

        public static string binaryenShellExecutable
        {
            get
            {
                var binaryenShell = Paths.Combine(binaryen, "bin", "wasm-opt");
                if (IsWindows())
                    binaryenShell += ".exe";
                return binaryenShell;
            }
        }

        public static string binaryenDisExecutable
        {
            get
            {
                var binaryenDis = Paths.Combine(binaryen, "bin", "wasm-dis");
                if (IsWindows())
                    binaryenDis += ".exe";
                return binaryenDis;
            }
        }

        public static string llvmDir
        {
            get
            {
                // Developer users may have activated a custom version of Emscripten SDK, in which case a LLVM_ROOT environment
                // variable provides the location of clang.exe etc.
                string existingLLvmRoot = Environment.GetEnvironmentVariable("LLVM_ROOT");
                if (!string.IsNullOrEmpty(existingLLvmRoot))
                    return existingLLvmRoot;

                string platform = "Unknown";
                if (IsWindows())
                    platform = "Win";
                else if (IsMac())
                    platform = "Mac";
                else if (IsLinux())
                    platform = "Linux";
                else
                    throw new Exception("Unknown platform");
                return Paths.Combine(buildToolsDir, "Emscripten_FastComp_" + platform);
            }
        }

        public static string emscriptenConfig
        {
            get
            {
                // Developer users may have activated a custom version of Emscripten SDK, in which case EM_CONFIG
                // environment variable locates the .emscripten configuration file
                string emsdkConfig = Environment.GetEnvironmentVariable("EM_CONFIG");
                if (!string.IsNullOrEmpty(emsdkConfig))
                {
                    return emsdkConfig;
                }

                return Paths.Combine(buildToolsDir, "emscripten.config");
            }
        }

        public static string tempDirForEmscriptenCompiler
        {
            get { return Paths.Combine(dataPath, "..", "Temp", "EmscriptenTemp"); }
        }

        public static string workingDirForEmscriptenCompiler
        {
            get { return Paths.Combine(dataPath, "..", "Temp", "EmscriptenWork"); }
        }

        public static string cacheDirForIl2CppIncrementalBuildArtifacts
        {
            get { return Paths.Combine(dataPath, "..", "Library"); }
        }

        public static string emscriptenCache
        {
            get { return Paths.Combine(llvmDir, "cache"); }
        }
    }
}
