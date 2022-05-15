using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using UnityEditor;
using UnityEditor.Modules;
using UnityEditor.WebGL.Emscripten;
using UnityEngine;

namespace UnityEditor.WebGL
{
    internal class WebGlSettingsExtension : DefaultPlayerSettingsEditorExtension
    {
        SerializedProperty m_CompressionFormat;
        SerializedProperty m_ExceptionSupport;
        SerializedProperty m_NameFilesAsHashes;
        SerializedProperty m_DataCaching;
        SerializedProperty m_DebugSymbols;
        SerializedProperty m_Template;
        SerializedProperty m_DefaultCanvasWidth;
        SerializedProperty m_DefaultCanvasHeight;
        SerializedProperty m_RunInBackground;
        SerializedProperty m_DecompressionFallback;
        SerializedProperty m_WasmArithmeticExceptions;

        WebGLTemplateManager m_TemplateManager = new WebGLTemplateManager();

        static GUIContent s_StacktraceWarning = EditorGUIUtility.TrTextContent("'Full With Stacktrace' exception support will decrease performance and increase browser memory usage. Only use this for debugging purposes, and make sure to test in a 64-bit browser.", EditorGUIUtility.GetHelpIcon(MessageType.Warning));

        override public void OnEnable(PlayerSettingsEditor settingsEditor)
        {
            base.OnEnable(settingsEditor);

            m_ExceptionSupport = settingsEditor.FindPropertyAssert("webGLExceptionSupport");
            m_CompressionFormat = settingsEditor.FindPropertyAssert("webGLCompressionFormat");
            m_NameFilesAsHashes = settingsEditor.FindPropertyAssert("webGLNameFilesAsHashes");
            m_DataCaching = settingsEditor.FindPropertyAssert("webGLDataCaching");
            m_DebugSymbols = settingsEditor.FindPropertyAssert("webGLDebugSymbols");
            m_Template = settingsEditor.FindPropertyAssert("webGLTemplate");
            m_DefaultCanvasWidth = settingsEditor.FindPropertyAssert("defaultScreenWidthWeb");
            m_DefaultCanvasHeight = settingsEditor.FindPropertyAssert("defaultScreenHeightWeb");
            m_RunInBackground = settingsEditor.FindPropertyAssert("runInBackground");
            m_DecompressionFallback = settingsEditor.FindPropertyAssert("webGLDecompressionFallback");
            m_WasmArithmeticExceptions = settingsEditor.FindPropertyAssert("webGLWasmArithmeticExceptions");
        }

        override public bool HasPublishSection()
        {
            return true;
        }

        override public void PublishSectionGUI(float h, float kLabelFloatMinW, float kLabelFloatMaxW)
        {
            EditorGUILayout.PropertyField(m_ExceptionSupport, EditorGUIUtility.TrTextContent("Enable Exceptions", "Turn this on to enable exception catching in WebGL. Don't use this if you don't need to as it has a significant performance overhead."));
            if (PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.FullWithStacktrace)
                EditorGUILayout.HelpBox(s_StacktraceWarning);
            EditorGUILayout.PropertyField(m_WasmArithmeticExceptions, EditorGUIUtility.TrTextContent("WebAssembly Arithmetic Exceptions", "WebAssembly code can throw an exception on things like division by zero, rounding a very large float to an int, and so forth. It is recommended to use Throw mode, as this will help to timely detect potential issues."));
            EditorGUILayout.PropertyField(m_CompressionFormat, EditorGUIUtility.TrTextContent("Compression Format", "Compression format for non-development build files. Note that this setting does not affect development builds, therefore its files will not be compressed."));
            EditorGUILayout.PropertyField(m_NameFilesAsHashes, EditorGUIUtility.TrTextContent("Name Files As Hashes", "Use MD5 hash of the uncompressed file contents as a filename for each file in the build."));
            EditorGUILayout.PropertyField(m_DataCaching, EditorGUIUtility.TrTextContent("Data Caching", "Automatically cache downloaded assets locally on users' machine to skip long downloads in subsequent runs."));
            EditorGUILayout.PropertyField(m_DebugSymbols, EditorGUIUtility.TrTextContent("Debug Symbols", "Preserve debug symbols and perform demangling of the stack trace when an error occurs. For release builds all the debug information is stored in a separate file which is downloaded from the server on demand when an error occurs. Development builds always have demangling support embedded in the main module and therefore are not affected by this option."));
            EditorGUILayout.PropertyField(m_DecompressionFallback, EditorGUIUtility.TrTextContent("Decompression Fallback", "Include decompression fallback code for build files in the loader. Use this option if you are not able to cofigure server response headers according to the selected compression method."));
        }

        override public bool HasResolutionSection()
        {
            return true;
        }

        override public void ResolutionSectionGUI(float h, float midWidth, float maxWidth)
        {
            GUILayout.Label(EditorGUIUtility.TrTextContent("Resolution"), EditorStyles.boldLabel);

            EditorGUI.BeginChangeCheck();
            EditorGUILayout.PropertyField(m_DefaultCanvasWidth, EditorGUIUtility.TrTextContent("Default Canvas Width*"));
            if (EditorGUI.EndChangeCheck() && m_DefaultCanvasWidth.intValue < 1)
                m_DefaultCanvasWidth.intValue = 1;

            EditorGUI.BeginChangeCheck();
            EditorGUILayout.PropertyField(m_DefaultCanvasHeight, EditorGUIUtility.TrTextContent("Default Canvas Height*"));
            if (EditorGUI.EndChangeCheck() && m_DefaultCanvasHeight.intValue < 1)
                m_DefaultCanvasHeight.intValue = 1;

            EditorGUILayout.PropertyField(m_RunInBackground, EditorGUIUtility.TrTextContent("Run In Background*"));

            EditorGUILayout.Space();

            GUILayout.Label(EditorGUIUtility.TrTextContent("WebGL Template"), EditorStyles.boldLabel);

            m_TemplateManager.SelectionUI(m_Template);
        }

        public override bool CanShowUnitySplashScreen() { return true; }
    }

    internal class WebGLTemplateManager : WebTemplateManagerBase
    {
        const string kWebTemplateDefaultIconResource = "BuildSettings.WebGL.Small";

        public override string customTemplatesFolder
        {
            get
            {
                return Path.Combine(Application.dataPath, "WebGLTemplates");
            }
        }

        public override string builtinTemplatesFolder
        {
            get
            {
                return Path.Combine(EmscriptenPaths.buildToolsDir, "WebGLTemplates");
            }
        }

        public override Texture2D defaultIcon
        {
            get
            {
                return (Texture2D)EditorGUIUtility.IconContent(kWebTemplateDefaultIconResource).image;
            }
        }

        public override string[] GetCustomKeys(string path)
        {
            Regex preprocessedFilenameRegex = new Regex("\\.(html|php|css|js|json)$");
            Regex outputRegex = new Regex("((^\\[|,)\"([a-zA-Z_$][a-zA-Z0-9_$]*)\")*\\]$");
            Regex variableRegex = new Regex("(?<=\")([a-zA-Z_$][a-zA-Z0-9_$]*)(?=\")");
            string[] ignoredVariables = new string[]
            {
                "COMPANY_NAME",
                "PRODUCT_NAME",
                "PRODUCT_VERSION",
                "WIDTH",
                "HEIGHT",
                "SPLASH_SCREEN_STYLE",
                "BACKGROUND_COLOR",
                "DATA_FILENAME",
                "FRAMEWORK_FILENAME",
                "CODE_FILENAME",
                "MEMORY_FILENAME",
                "SYMBOLS_FILENAME",
                "BACKGROUND_FILENAME",
                "USE_WASM",
                "USE_THREADS",
                "USE_WEBGL_1_0",
                "USE_WEBGL_2_0",
                "USE_DATA_CACHING",
                "DECOMPRESSION_FALLBACK",
                "TOTAL_MEMORY",
                "DEVELOPMENT_PLAYER",
                "UNITY_VERSION",
                "LOADER_FILENAME",
            };
            List<string> variables = new List<string>();

            foreach (var file in FileUtil.GetAllFilesRecursive(path))
            {
                if (preprocessedFilenameRegex.IsMatch(FileUtil.UnityGetFileName(file)))
                {
                    var sb = new StringBuilder();
                    foreach (char c in file)
                    {
                        if (('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == ' ' || c == '.' || c == '/')
                            sb.Append(c);
                        else
                            sb.Append("\\" + (c < 0x100 ? "x" : "u") + ((int)c).ToString(c < 0x100 ? "x2" : "x4"));
                    }
                    var processStartInfo = new ProcessStartInfo(EmscriptenPaths.nodeExecutable)
                    {
                        Arguments = $"Preprocess.js \"locals.inputPath='{sb.ToString()}'\"",
                        WorkingDirectory = EmscriptenPaths.buildToolsDir,
                        UseShellExecute = false,
                        CreateNoWindow = true,
                        RedirectStandardOutput = true,
                        RedirectStandardError = true,
                    };
                    var stdout = new StringBuilder();
                    var stderr = new StringBuilder();
                    using (var process = Process.Start(processStartInfo))
                    {
                        process.OutputDataReceived += (sender, e) => { stdout.Append(e.Data); };
                        process.ErrorDataReceived += (sender, e) => { stderr.Append(e.Data); };
                        process.BeginOutputReadLine();
                        process.BeginErrorReadLine();
                        process.WaitForExit();
                        var output = stdout.ToString();
                        if (process.ExitCode != 0)
                        {
                            UnityEngine.Debug.LogError("Failed running " + processStartInfo.FileName + " " + processStartInfo.Arguments + "\n\nstdout:" + stdout + "\nstderr:" + stderr);
                        }
                        else if (!outputRegex.IsMatch(output))
                        {
                            UnityEngine.Debug.LogError("Invalid output " + processStartInfo.FileName + " " + processStartInfo.Arguments + "\n\nstdout:" + output);
                        }
                        else
                        {
                            foreach (Match variableMatch in variableRegex.Matches(output))
                            {
                                if (!ignoredVariables.Contains(variableMatch.Value) && !variables.Contains(variableMatch.Value))
                                    variables.Add(variableMatch.Value);
                            }
                        }
                    }
                }
            }
            return variables.ToArray();
        }
    }
}
