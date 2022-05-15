using System.IO;
using System.Collections.Generic;
using System.Linq;
using UnityEditor.WebGL.Emscripten;
using UnityEditorInternal;

namespace UnityEditor.WebGL.Il2Cpp
{
    public class EmscriptenCompiler
    {
        public const string LinkerFlags = "-Oz -s NO_EXIT_RUNTIME=1";

        public static string GetCompilerFlags(bool exceptionSupport)
        {
            return "-Oz" + (!exceptionSupport ? " -DIL2CPP_EXCEPTION_DISABLED=1 " : "");
        }

        public static IEnumerable<string> GetIncludeFullPaths(IEnumerable<string> includePaths)
        {
            EmscriptenPaths.SetupDataPath();
            return includePaths.Select(source => Path.Combine(IL2CPPBuilder.GetShortPathName(Path.GetFullPath(EmscriptenPaths.dataPath + "/../")), source));
        }

        public static string GetOutFileFullPath(string outFileRelativePath)
        {
            EmscriptenPaths.SetupDataPath();
            return Path.Combine(IL2CPPBuilder.GetShortPathName(Path.GetFullPath(EmscriptenPaths.dataPath + "/../")), outFileRelativePath);
        }

        public static void CleanupAndCreateEmscriptenDirs()
        {
            EmscriptenPaths.SetupDataPath();
            if (Directory.Exists(EmscriptenPaths.tempDirForEmscriptenCompiler))
                Directory.Delete(EmscriptenPaths.tempDirForEmscriptenCompiler, true);

            if (Directory.Exists(EmscriptenPaths.workingDirForEmscriptenCompiler))
                Directory.Delete(EmscriptenPaths.workingDirForEmscriptenCompiler, true);

            Directory.CreateDirectory(EmscriptenPaths.tempDirForEmscriptenCompiler);
            Directory.CreateDirectory(EmscriptenPaths.workingDirForEmscriptenCompiler);
        }
    }
}
