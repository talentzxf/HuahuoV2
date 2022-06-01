using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using UnityEditor.WebGL.Emscripten;
using UnityEditorInternal;
using UnityEditor.Build.Reporting;
using UnityEditor.WebGL.UnityLinker;

namespace UnityEditor.WebGL.Il2Cpp
{
    internal class WebGlIl2CppPlatformProvider : BaseIl2CppPlatformProvider
    {
        private readonly string m_NativeLibraryFileName;
        public string LinkerFlags = "";
        public IEnumerable<string> Libs = new string[0];
        public IEnumerable<string> JsPre = new string[0];
        public IEnumerable<string> JsLib = new string[0];

        public WebGlIl2CppPlatformProvider(BuildTarget target, string dataDirectory, string nativeLibraryFileName,
                                           BuildReport _buildReport, string baselibLibraryDirectory)
            : base(target, Path.Combine(dataDirectory, "Libraries"), _buildReport, baselibLibraryDirectory)
        {
            m_NativeLibraryFileName = nativeLibraryFileName;
        }

        public override bool emitNullChecks
        {
            get { return PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.FullWithStacktrace || PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.FullWithoutStacktrace; }
        }

        public override bool enableStackTraces
        {
            get { return PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.FullWithStacktrace; }
        }

        public override bool enableArrayBoundsCheck
        {
            get { return PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.FullWithStacktrace || PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.FullWithoutStacktrace; }
        }

        public override bool allowDebugging
        {
            // Script debugging is not supported on WebGL platform.
            get { return false; }
        }

        public override string[] libraryPaths
        {
            get { return new string[0]; }
        }

        public override string[] includePaths
        {
            get { return new string[0]; }
        }

        public override string nativeLibraryFileName
        {
            get { return m_NativeLibraryFileName; }
        }

        public override Il2CppNativeCodeBuilder CreateIl2CppNativeCodeBuilder()
        {
            return new WebGLIl2CppNativeCodeBuilder(
                PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.FullWithStacktrace ||
                PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.FullWithoutStacktrace,
                LinkerFlags,
                Libs,
                JsPre,
                JsLib,
                baselibLibraryDirectory);
        }

        public override BaseUnityLinkerPlatformProvider CreateUnityLinkerPlatformProvider()
        {
            return new WebGLUnityLinkerPlatformProvider(target);
        }
    }
}
