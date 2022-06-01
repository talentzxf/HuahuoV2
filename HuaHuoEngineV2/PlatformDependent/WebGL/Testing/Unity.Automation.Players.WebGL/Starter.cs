using Unity.CommonTools;
using UnityEngine.Common;
using UnityEngine.Common.Enums;
using UnityEngine.Common.PlayerConnection;

namespace Unity.Automation.Players.WebGL
{
    [AutomationSupportFor("UnityWebGLPlayer")]
    public class Starter : BrowserHostablePlayerStartInfo
    {
        private readonly IPlayerConnectionMessageTranslator m_MessageTranslator;

        public Starter(IPlayerConnectionMessageTranslator messageTranslator)
            : base(RuntimePlatform.WebGL, GetPlatformCapabilities())
        {
            m_MessageTranslator = messageTranslator;
            LogToConsole = true;

            // We use Firefox for WebGL tests.
            WebBrowserStartInfo.BrowserType = WebBrowserType.Firefox;
        }

        private static IPlatformCapabilities GetPlatformCapabilities()
        {
            return new PlatformCapabilities
            {
                SupportsTerrain = true,
                SupportsCloth = true,
                SupportsLinearLighting = true,

                SupportsPlayerConnection = true,
            };
        }

        protected override bool ReadLogTroughPlayerConnectionByDefault()
        {
            return true;
        }

        public override string ContentDirectory => Paths.Combine(ProjectDirectory, BuildName);

        protected override string TargetBuildPath => Paths.Combine(ProjectDirectory, BuildName);

        public override string ApplicationPath => Paths.Combine(ProjectDirectory, BuildName, "index.html");

        public override IUnityProcessBase Start()
        {
            if (!string.IsNullOrEmpty(DeviceId))
                WebBrowserStartInfo.BrowserType = (WebBrowserType)System.Enum.Parse(typeof(WebBrowserType), DeviceId);
            return new Player(m_MessageTranslator, this);
        }
    }
}
