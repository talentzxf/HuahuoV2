using System;
using UnityEditor.Modules;
using UnityEditorInternal;

namespace UnityEditor.WebGL
{
    internal class TargetExtension : DefaultPlatformSupportModule
    {
        private static WebGlBuildWindowExtension s_BuildWindow;

        public override string TargetName { get { return "WebGL"; } }
        public override string JamTarget { get { return "WebGLExtensions"; } }

        public override void OnLoad()
        {
            SysrootManager.Initialize();
        }

        public override IBuildPostprocessor CreateBuildPostprocessor()
        {
            return new WebGlBuildPostprocessor();
        }

        public override ISettingEditorExtension CreateSettingsEditorExtension()
        {
            return new WebGlSettingsExtension();
        }

        public override IBuildWindowExtension CreateBuildWindowExtension()
        {
            return s_BuildWindow ?? (s_BuildWindow = new WebGlBuildWindowExtension());
        }

        public override IUserAssembliesValidator CreateUserAssembliesValidatorExtension()
        {
#if ENABLE_WEBGL_ASSEMBLY_VALIDATION
            return new WebGLAssembliesValidator();
#else
            return null;
#endif
        }
    }
}
