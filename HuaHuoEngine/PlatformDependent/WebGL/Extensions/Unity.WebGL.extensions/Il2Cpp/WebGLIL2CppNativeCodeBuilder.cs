using System.Collections.Generic;
using System.Diagnostics;
using UnityEditor.WebGL.Emscripten;
using UnityEditorInternal;

namespace UnityEditor.WebGL.Il2Cpp
{
    public class WebGLIl2CppNativeCodeBuilder : Il2CppNativeCodeBuilder
    {
        private readonly bool _enableExceptionSupport;
        private readonly string _linkerFlags = EmscriptenCompiler.LinkerFlags;
        private readonly IEnumerable<string> _libs;
        private readonly IEnumerable<string> _jsPre;
        private readonly IEnumerable<string> _jsLib;

        public WebGLIl2CppNativeCodeBuilder(bool enableExceptionSupport)
            : base(null)
        {
            _enableExceptionSupport = enableExceptionSupport;
        }

        public WebGLIl2CppNativeCodeBuilder(bool enableExceptionSupport, string linkerFlags, IEnumerable<string> libs,
                                            IEnumerable<string> jsPre, IEnumerable<string> jsLib, string baselibLibraryDirectory)
            : base(baselibLibraryDirectory)
        {
            _enableExceptionSupport = enableExceptionSupport;
            _linkerFlags = linkerFlags;
            _libs = libs;
            _jsPre = jsPre;
            _jsLib = jsLib;
        }

        public override IEnumerable<string> AdditionalIl2CPPArguments
        {
            get
            {
                var sysroot = SysrootManager.FindSysroot(BuildTarget.WebGL);
                if (sysroot != null)
                {
                    foreach (string arg in sysroot.GetIl2CppArguments())
                    {
                        yield return arg;
                    }
                }

                yield return "--emit-method-map";

                // We currently can't support C# multi-threading due to garbage collection
                // not being supported in threads.
                //if (PlayerSettings.WebGL.threadsSupport)
                //yield return "--additional-defines=IL2CPP_SUPPORT_THREADS";

                foreach (var jsPre in _jsPre)
                    yield return "--js-pre=\"" + jsPre + "\"";

                foreach (var jsLib in _jsLib)
                    yield return "--js-libraries=\"" + jsLib + "\"";

                foreach (var lib in _libs)
                {
                    var extension = System.IO.Path.GetExtension(lib);
                    if (extension == ".c" || extension == ".cc" || extension == ".cpp" || extension == ".a"  || extension == ".bc")
                    {
                        yield return "--additional-libraries=\"" + lib + "\"";
                    }
                    else
                    {
                        UnityEngine.Debug.LogWarning("Plugin " + lib + " not supported on WebGL.");
                    }
                }
            }
        }

        public override string CompilerPlatform
        {
            get { return "WebGL"; }
        }

        public override string CompilerArchitecture
        {
            get { return "EmscriptenJavaScript"; }
        }

        public override bool SetsUpEnvironment
        {
            get { return true; }
        }

        public override string CacheDirectory
        {
            get { return EmscriptenPaths.cacheDirForIl2CppIncrementalBuildArtifacts; }
        }

        public override string CompilerFlags
        {
            get { return EmscriptenCompiler.GetCompilerFlags(_enableExceptionSupport); }
        }

        public override string LinkerFlags
        {
            get { return _linkerFlags; }
        }

        public override IEnumerable<string> ConvertIncludesToFullPaths(IEnumerable<string> relativeIncludePaths)
        {
            return EmscriptenCompiler.GetIncludeFullPaths(relativeIncludePaths);
        }

        public override string ConvertOutputFileToFullPath(string outputFileRelativePath)
        {
            return EmscriptenCompiler.GetOutFileFullPath(outputFileRelativePath);
        }

        protected override void SetupEnvironment(ProcessStartInfo startInfo)
        {
            EmccArguments.SetupDefaultEmscriptenEnvironment(startInfo);
        }
    }
}
