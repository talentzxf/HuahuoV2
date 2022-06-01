using System;
using UnityEditor;
using UnityEditorInternal;

namespace UnityEditor.WebGL.UnityLinker
{
    internal class WebGLUnityLinkerPlatformProvider : BaseUnityLinkerPlatformProvider
    {
        public WebGLUnityLinkerPlatformProvider(BuildTarget target)
            : base(target)
        {
        }

        public override string Platform => "WebGL";

        public override string Architecture => "EmscriptenJavaScript";
    }
}
