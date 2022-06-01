using Unity.CommonTools;
using UnityEngine.Common;
using System;
using UnityEngine.Common.PlayerConnection;

namespace Unity.Automation.Players.WebGL
{
    public class Player : BrowserHostedPlayer
    {
        PlayerConnectionWebsockifyWrapper websockifyWrapper;

        public Player(IPlayerConnectionMessageTranslator messageTranslator, Starter startInfo)
            : base(messageTranslator, startInfo)
        {
            Launch();
        }

        protected override IPlayerConnection CreatePlayerConnection(object device)
        {
            websockifyWrapper = new PlayerConnectionWebsockifyWrapper();
            return base.CreatePlayerConnection(device);
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (websockifyWrapper != null)
                    websockifyWrapper.Dispose();
            }
            base.Dispose(disposing);
        }

        protected override void SetupEnvironmentVariables(UnityProcessBaseStartInfo startInfo)
        {
            var envVars = _browser.EnvironmentVariables;
            SetupUnityEnvironmentVarsForProxyServer(envVars, startInfo.WebProxy);
            foreach (var kvp in startInfo.EnvironmentVariables)
            {
                envVars.Add(kvp.Key, kvp.Value);
            }

            envVars.Add("UNITY_KEEP_LOG_FILES", "1");
            envVars.Add(startInfo.UseCleanLog ? "UNITY_CLEANED_LOG_FILE" : "UNITY_LOG_FILE", LogFilePath);

            if (!startInfo.KeepMonoEnvironmentVariables)
            {
                Files.RemoveMonoEnvironmentVariables(_browser.StartInfo);
            }
        }

        public override TimeSpan GetReasonableTimeToExpectFirstLogMessage()
        {
            return TimeSpan.FromSeconds(400);
        }
    }
}
