using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;

namespace UnityEditor.WebGL.Emscripten
{
    internal class EmccArguments
    {
        internal static void SetupDefaultEmscriptenEnvironment(ProcessStartInfo startInfo)
        {
            // Developer users may have activated a custom version of Emscripten SDK, in which case EMSDK
            // environment variable locates the root directory of the SDK, and we should not do any custom
            // preparation of Emscripten environment variables.
            string existingEmsdk = Environment.GetEnvironmentVariable("EMSDK");
            if (!string.IsNullOrEmpty(existingEmsdk))
            {
                UnityEngine.Debug.Log("Using existing developer provided Emscripten SDK installation from '" + existingEmsdk + "'");
                return;
            }

            FixClangSymLinkOnMac();
            SetEnvironmentVariable(startInfo, "EM_CONFIG", EmscriptenPaths.emscriptenConfig);
            SetEnvironmentVariable(startInfo, "LLVM", EmscriptenPaths.llvmDir);
            SetEnvironmentVariable(startInfo, "NODE", EmscriptenPaths.nodeExecutable);
            SetEnvironmentVariable(startInfo, "EMSCRIPTEN", EmscriptenPaths.emscriptenDir);
            SetEnvironmentVariable(startInfo, "EMSCRIPTEN_TMP", EmscriptenPaths.tempDirForEmscriptenCompiler);
            SetEnvironmentVariable(startInfo, "EM_CACHE", EmscriptenPaths.emscriptenCache);
            SetEnvironmentVariable(startInfo, "EMSCRIPTEN_NATIVE_OPTIMIZER", EmscriptenPaths.optimizer);
            SetEnvironmentVariable(startInfo, "BINARYEN", EmscriptenPaths.binaryen);
            SetEnvironmentVariable(startInfo, "EMCC_SKIP_SANITY_CHECK", "1");
            SetEnvironmentVariable(startInfo, "EMCC_WASM_BACKEND", "0");
            SetEnvironmentVariable(startInfo, "EM_EXCLUSIVE_CACHE_ACCESS", "1");
        }

        private static string SetEnvironmentVariable(ProcessStartInfo startInfo, string environmentVariable, string value)
        {
            if (debugEnvironmentAndInvocations)
                UnityEngine.Debug.Log(string.Format("SET {0}={1}", environmentVariable, value));

            return startInfo.EnvironmentVariables[environmentVariable] = value;
        }

        public static bool debugEnvironmentAndInvocations
        {
            get { return false; }
        }

        private static void FixClangSymLinkOnMac()
        {
            if (!EmscriptenPaths.IsMac())
                return;

            FixSymlinkIfNecessary("/clang++");
            FixSymlinkIfNecessary("/clang");
        }

        private static void FixSymlinkIfNecessary(string binary)
        {
            if (new FileInfo(EmscriptenPaths.llvmDir + binary).Length != 0)
                return;

            var process = Process.Start("ln", "-sf clang-3.3 " + EmscriptenPaths.llvmDir + binary);
            if (process != null)
                process.WaitForExit();
        }
    }
}
