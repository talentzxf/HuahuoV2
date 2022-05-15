namespace UnityEditor.WebGL
{
    public enum CodeOptimization
    {
        Speed,
        Size,
    }

    public static class UserBuildSettings
    {
        private readonly static string kCodeOptimization = "CodeOptimization";

        public static CodeOptimization codeOptimization
        {
            get
            {
                switch (EditorUserBuildSettings.GetPlatformSettings(BuildPipeline.GetBuildTargetName(BuildTarget.WebGL), kCodeOptimization).ToLower())
                {
                    case "size":
                        return CodeOptimization.Size;
                    case "speed":
                    default:
                        return CodeOptimization.Speed;
                }
            }
            set
            {
                EditorUserBuildSettings.SetPlatformSettings(BuildPipeline.GetBuildTargetName(BuildTarget.WebGL), kCodeOptimization, value.ToString());
            }
        }
    }
}
