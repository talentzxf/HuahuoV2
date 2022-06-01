using System;
using System.IO;
using UnityEngine;
using UnityEditor;
using UnityEditor.Modules;

namespace UnityEditor.WebGL
{
    internal class WebGlBuildWindowExtension : DefaultBuildWindowExtension
    {
        private static readonly GUIContent debugBuild = EditorGUIUtility.TrTextContent("Development Build");
        private static readonly GUIContent codeOptimization = EditorGUIUtility.TrTextContent("Code Optimization", "Optimization that will be used when compiling WebGL code.");

        private static readonly CodeOptimization[] codeOptimizations =
        {
            CodeOptimization.Speed,
            CodeOptimization.Size,
        };

        private static readonly GUIContent[] codeOptimizationStrings =
        {
            EditorGUIUtility.TrTextContent("Speed"),
            EditorGUIUtility.TrTextContent("Size"),
        };

        bool Is64Bit()
        {
            return IntPtr.Size == 8;
        }

        public override void ShowPlatformBuildOptions()
        {
            if (!Is64Bit())
            {
                GUILayout.BeginVertical(EditorStyles.helpBox);
                GUILayout.Label("Building for WebGL requires a 64-bit Unity editor.", EditorStyles.wordWrappedMiniLabel);
                GUILayout.EndVertical();
                return;
            }

            EditorUserBuildSettings.development = EditorGUILayout.Toggle(debugBuild, EditorUserBuildSettings.development);
            if (EditorUserBuildSettings.development)
            {
                GUILayout.BeginVertical(EditorStyles.helpBox);
                GUILayout.Label("Note that WebGL development builds are much larger than release builds and should not be published.", EditorStyles.wordWrappedMiniLabel);
                GUILayout.EndVertical();
            }

            if (!EditorUserBuildSettings.development)
            {
                int selectedIndex = Math.Max(0, Array.IndexOf(codeOptimizations, UserBuildSettings.codeOptimization));
                selectedIndex = EditorGUILayout.Popup(codeOptimization, selectedIndex, codeOptimizationStrings);
                UserBuildSettings.codeOptimization = codeOptimizations[selectedIndex];
            }
        }

        public override bool EnabledBuildButton()
        {
            return Is64Bit();
        }

        public override bool EnabledBuildAndRunButton()
        {
            return EnabledBuildButton();
        }

        public override bool ShouldDrawScriptDebuggingCheckbox()
        {
            return false;
        }

        public override bool ShouldDrawProfilerCheckbox()
        {
            return Is64Bit();
        }

        public override bool ShouldDrawDevelopmentPlayerCheckbox()
        {
            return false;
        }
    }
}
