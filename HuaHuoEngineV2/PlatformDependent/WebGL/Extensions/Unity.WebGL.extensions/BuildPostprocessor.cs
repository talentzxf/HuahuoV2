using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text.RegularExpressions;
using UnityEditor.Build.Reporting;
using UnityEditor.Modules;
using UnityEditor.Utils;
using UnityEditor.WebGL.Emscripten;
using UnityEditor.WebGL.Il2Cpp;
using UnityEditorInternal;
using UnityEngine;
using Debug = UnityEngine.Debug;

namespace UnityEditor.WebGL
{
    internal class WebGlBuildPostprocessor : DefaultBuildPostprocessor
    {
        private const string kUnityLoaderFileType = "Build Loader";
        private const string kUnityLoaderPath = "UnityLoader.js";

        private const string kDataFileType = "Application Data";

        private const string kWasmFrameworkFileType = "WebAssembly Framework";
        private const string kAsmFrameworkFileType = "asm.js Framework";
        private const string kFrameworkNativeExtension = ".js";
        private const string kFrameworkFunctionName = "unityFramework";

        private const string kWasmCodeFileType = "WebAssembly Code";
        private const string kWasmCodeNativeExtension = ".wasm";

        private const string kWasmSymbolsFileType = "WebAssembly Debug Symbols";
        private const string kWasmSymbolsNativeExtension = ".js.symbols";

        private const string kAsmCodeFileType = "asm.js Code";
        private const string kAsmCodeNativeExtension = ".asm.js";

        private const string kAsmSymbolsFileType = "asm.js Debug Symbols";
        private const string kAsmSymbolsNativeExtension = ".js.symbols";
        private const string kAsmSymbolsStrippedExtension = ".js.symbols.stripped";

        private const string kAsmMemoryFileType = "asm.js Memory Initializer";
        private const string kAsmMemorySuffix = ".asm.memory";
        private const string kWasmMemoryFileType = "WebAssembly Memory Initializer";
        private const string kWasmMemorySuffix = ".wasm.memory";
        private const string kMemoryNativeExtension = ".js.mem";

        private const string kAsmDynamicLibraryFileType = "asm.js Dynamic Library";
        private const string kAsmDynamicLibrarySuffix = ".asm.library";

        private const string kBackgroundFileType = "Build Background";
        private const string kBackgroundExtension = ".jpg";

        private const string kTemplateFileType = "WebGL Template";

        private const string kBitcodeExtension = ".bc";
        private const string kBuildExtension = ".unityweb";
        private const string kGzipExtension = ".gz";
        private const string kBrotliExtension = ".br";
        private const string kCompressedExtension = ".compressed";
        private const string kCompressionMarkerGzip = "UnityWeb Compressed Content (gzip)";
        private const string kCompressionMarkerBrotli = "UnityWeb Compressed Content (brotli)";

        private const string kNativeFilename = "build";
        private const string kNativeFolder = "Native";
        private const string kBuildFolder = "Build";
        private const string kOutputFolder = "Output";
        private const string kManagedFolder = "Managed";
        private const string kResourcesFolder = "Resources";
        private const string kIl2CppDataFolder = "Il2CppData";
        private const string kWebGLCache = "webgl_cache";
        private const string kAsmLinkResult = "linkresult_asm";
        private const string kWasmLinkResult = "linkresult_wasm";

        private const string kJsPluginExtension = ".js";
        private const string kJslibPluginExtension = ".jslib";
        private const string kJsprePluginExtension = ".jspre";
        private string[] m_NativePluginsExtensions = new string[] {".cpp", ".cc", ".c", ".bc", ".a"};

        private string m_WebGLCache;
        private string m_StagingAreaData;
        private string m_StagingAreaDataOutput;
        private string m_StagingAreaDataOutputBuild;
        private string m_StagingAreaDataNative;
        private string m_StagingAreaDataManaged;
        private string m_StagingAreaDataResources;
        private string m_StagingAreaDataIl2CppData;

        private bool m_DevelopmentPlayer;
        private bool m_UseWasm;
        private bool m_NameFilesAsHashes;
        private string m_BuildName;
        private string m_TotalMemory;
        private string m_TemplatePath;
        private Dictionary<string, string> m_BuildFiles;

        private WebGlIl2CppPlatformProvider m_PlatformProvider;
        private RuntimeClassRegistry m_RCR;

        private readonly ProgressHelper m_Progress = new ProgressHelper();
        private HashAlgorithm m_Hash = MD5.Create();

        private readonly string[] m_ExtraExportedRuntimeMethods = new string[]
        {
            "addRunDependency",
            "removeRunDependency",
            "FS_createPath",
            "FS_createDataFile",
            "ccall",
            "cwrap",
            "stackTrace"
        };

        private struct DataFile
        {
            public string path;
            public long length;
            public byte[] internalPath;

            public DataFile(string path, string internalPath)
            {
                this.path = path;
                this.length = new System.IO.FileInfo(path).Length;
                this.internalPath = System.Text.Encoding.UTF8.GetBytes(internalPath.Replace('\\', '/'));
            }
        }

        private void BuildStep(BuildPostProcessArgs args, string title, string description)
        {
            m_Progress.Show(title, description);
            args.report.BeginBuildStep(description);
        }

        private string ComputeHashString(byte[] data)
        {
            return string.Join(string.Empty, System.Array.ConvertAll(m_Hash.ComputeHash(data), b => b.ToString("X2")));
        }

        private string ComputeHashString(string s)
        {
            return ComputeHashString(System.Text.Encoding.ASCII.GetBytes(s));
        }

        private string ComputeHashString(bool b)
        {
            return ComputeHashString(b.ToString());
        }

        private string PreprocessorArgument(string variable, string value)
        {
            var sb = new System.Text.StringBuilder();
            foreach (char c in value)
            {
                if (('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == ' ' || c == '.' || c == '/')
                    sb.Append(c);
                else
                    sb.Append("\\" + (c < 0x100 ? "x" : "u") + ((int)c).ToString(c < 0x100 ? "x2" : "x4"));
            }
            return $" \"{variable}='{sb.ToString()}'\"";
        }

        private string PreprocessorArgument(string variable, bool value)
        {
            return $" \"{variable}={(value ? "true" : "false")}\"";
        }

        private string PreprocessorArgument(string variable, int value)
        {
            return $" \"{variable}={value}\"";
        }

        private void CheckBuildPrerequisites()
        {
            if (!Directory.Exists(m_TemplatePath))
                throw new System.Exception("Invalid WebGL template path: " + m_TemplatePath + "! Select a template in player settings.");
            if (!File.Exists(Paths.Combine(m_TemplatePath, "index.html")))
                throw new System.Exception("Invalid WebGL template selection: " + m_TemplatePath + "! \"index.html\" file not found.");
        }

        private string[] GetModules()
        {
            List<string> modules = new List<string>();
            var modulePath = PlayerSettings.WebGL.modulesDirectory;
            if (string.IsNullOrEmpty(modulePath))
            {
                modulePath = Paths.Combine(EmscriptenPaths.buildToolsDir, "lib", "modules");
                if (m_DevelopmentPlayer)
                    modulePath += "_development";
                if (PlayerSettings.WebGL.threadsSupport)
                    modulePath += "_mt";
                if (!m_DevelopmentPlayer && UserBuildSettings.codeOptimization == CodeOptimization.Size)
                    modulePath += "_optsize";
            }
            else
            {
                if (!Path.IsPathRooted(modulePath))
                    modulePath = Paths.Combine(EmscriptenPaths.buildToolsDir, modulePath);
            }
            modules.AddRange(Directory.GetFiles(modulePath, "*" + kBitcodeExtension));
            return modules.ToArray();
        }

        private string[] GetJsprePlugins()
        {
            List<string> jsprePlugins = new List<string>();
            jsprePlugins.Add(Paths.Combine(EmscriptenPaths.buildToolsDir, "UserJsprePlaceholder.js"));
            var prejsPath = Paths.Combine(EmscriptenPaths.buildToolsDir, "prejs");
            foreach (var pre in Directory.GetFiles(prejsPath, "*" + kJsPluginExtension))
                jsprePlugins.Add(Paths.Combine(prejsPath, Path.GetFileName(pre)));
            if (PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.None)
                jsprePlugins.Add(Paths.Combine(EmscriptenPaths.buildToolsDir, "ExceptionLogger.js"));
            return jsprePlugins.ToArray();
        }

        private string[] GetJslibPlugins(BuildPostProcessArgs args)
        {
            List<string> jslibPlugins = new List<string>();

            // internal JS libs
            var libPath = Paths.Combine(EmscriptenPaths.buildToolsDir, "lib");
            foreach (var lib in Directory.GetFiles(libPath, "*" + kJsPluginExtension))
                jslibPlugins.Add(Paths.Combine(libPath, Path.GetFileName(lib)));

            // project's JS libs
            foreach (PluginImporter imp in PluginImporter.GetImporters(args.target))
            {
                if (imp.isNativePlugin && Path.GetExtension(imp.assetPath) == kJslibPluginExtension)
                    jslibPlugins.Add(EmscriptenPaths.GetShortPathName(Path.GetFullPath(imp.assetPath)));
            }
            return jslibPlugins.ToArray();
        }

        private string[] GetNativePlugins(BuildPostProcessArgs args)
        {
            List<string> nativePlugins = new List<string>();
            foreach (PluginImporter imp in PluginImporter.GetImporters(args.target))
            {
                if (imp.isNativePlugin && m_NativePluginsExtensions.Contains(Path.GetExtension(imp.assetPath)))
                    nativePlugins.Add(EmscriptenPaths.GetShortPathName(Path.GetFullPath(imp.assetPath)));
            }
            return nativePlugins.ToArray();
        }

        private string ArgumentsForEmscripten(bool wasmBuild)
        {
            // This needs to be in synch with the command line used for the prebuilt JS in WebGLSupport.jam!
            var argumentsForEmscripten = $" -O{(!m_DevelopmentPlayer && UserBuildSettings.codeOptimization == CodeOptimization.Size ? "s" : "3")}";
            argumentsForEmscripten += $" -g{(m_DevelopmentPlayer ? 2 : 0)}";

            argumentsForEmscripten += " -DUNITY_WEBGL=1";

            argumentsForEmscripten += " -s PRECISE_F32=2";
            argumentsForEmscripten += " -s USE_WEBGL2=1";
            argumentsForEmscripten += " -s FULL_ES3=1";
            argumentsForEmscripten += $" -s \"EXTRA_EXPORTED_RUNTIME_METHODS=['{string.Join("','", m_ExtraExportedRuntimeMethods)}']\"";
            argumentsForEmscripten += $" -s DISABLE_EXCEPTION_CATCHING={(PlayerSettings.WebGL.exceptionSupport == WebGLExceptionSupport.None ? 1 : 0)}";
            argumentsForEmscripten += $" -s TOTAL_MEMORY={((wasmBuild && !PlayerSettings.WebGL.threadsSupport) ? "32MB" : m_TotalMemory)}";

            if (m_DevelopmentPlayer)
            {
                argumentsForEmscripten += " -s ASSERTIONS=1";
                argumentsForEmscripten += " -s DEMANGLE_SUPPORT=1";
                argumentsForEmscripten += " -s ERROR_ON_UNDEFINED_SYMBOLS=1";
            }

            if (PlayerSettings.WebGL.threadsSupport)
            {
                argumentsForEmscripten += " -s USE_PTHREADS=1";
                argumentsForEmscripten += " -s PTHREAD_POOL_SIZE=8";
                argumentsForEmscripten += " -s PTHREAD_HINT_NUM_CORES=4"; // hint for browsers that don't support navigator.hardwareConcurrency (e.g. Safari)
                argumentsForEmscripten += " -s PROXY_TO_PTHREAD=0";
                argumentsForEmscripten += " -s TEXTDECODER=0";

                // unit tests can only run with proxying enabled, otherwise the browser tab will hang
                argumentsForEmscripten += $" -s PROXY_TO_PTHREAD={(PlayerSettings.WebGL.modulesDirectory == "lib/modules_testing_mt" ? 1 : 0)}";
            }

            argumentsForEmscripten += $" -s WASM={(wasmBuild ? 1 : 0)}";
            if (wasmBuild)
            {
                argumentsForEmscripten += $" -s \"BINARYEN_TRAP_MODE='{(PlayerSettings.WebGL.wasmArithmeticExceptions == WebGLWasmArithmeticExceptions.Throw ? "allow" : "clamp")}'\"";
                argumentsForEmscripten += " -s ALLOW_MEMORY_GROWTH=1";

                // WASM_MEM_MAX is a requirement for using pthreads
                if (PlayerSettings.WebGL.threadsSupport)
                    argumentsForEmscripten += $" -s WASM_MEM_MAX={m_TotalMemory}";
                // TODO: consider adding an option so users can cap the heap size in single-threaded builds too
            }
            else
            {
                argumentsForEmscripten += " --separate-asm";
                argumentsForEmscripten += " -s ASM_MODULE_NAME=\"var UnityAsmCode\"";
            }

            if (PlayerSettings.WebGL.threadsSupport)
            {
                argumentsForEmscripten += " -s MODULARIZE=1";
                argumentsForEmscripten += $" -s EXPORT_NAME={kFrameworkFunctionName}";
            }

            argumentsForEmscripten += " --memory-init-file 1";
            argumentsForEmscripten += " --emit-symbol-map";

            // Set linux line endings, so we get a consistent output format on all OSes.
            argumentsForEmscripten += " --output_eol linux";

            // INLINING_LIMIT=1 and llvm-lto 1 produces a smaller build which is faster in every use case we tried, however needs more proof to be enabled!
            //argumentsForEmscripten += "-s INLINING_LIMIT=1 ";
            //argumentsForEmscripten += "--llvm-lto 1 ";
            //TODO: investigate --llvm_opts =3

            argumentsForEmscripten += " " + PlayerSettings.WebGL.emscriptenArgs;

            return argumentsForEmscripten;
        }

        public override void LaunchPlayer(BuildLaunchPlayerArgs args)
        {
            int port;
            HttpServerEditorWrapper.CreateIfNeeded(args.installPath, out port);
            Application.OpenURL("http://localhost:" + port + "/");
        }

        private static UnityType FindTypeByNameChecked(string name)
        {
            UnityType result = UnityType.FindTypeByName(name);
            if (result == null)
                throw new System.ArgumentException(string.Format("Could not map typename '{0}' to type info (WebGL class registration skipped classes)", name));
            return result;
        }

        private void ModifyIl2CppOutputDirBeforeCompile(string outputDir)
        {
            File.Copy(Paths.Combine(m_StagingAreaDataManaged, "UnityICallRegistration.cpp"), Paths.Combine(outputDir, "UnityICallRegistration.cpp"));

            // These classes only exist if ENABLE_NETWORK is true, but it is false for WebGL, so skip registration of them.
            var classesToSkip = new UnityType[]
            {
                FindTypeByNameChecked("ClusterInputManager"),
                FindTypeByNameChecked("MovieTexture"),
            };
            UnityEditor.CodeStrippingUtils.WriteModuleAndClassRegistrationFile(Path.GetFullPath(m_StagingAreaDataManaged),
                Path.GetFullPath(Paths.Combine(m_StagingAreaDataManaged, "ICallSummary.txt")), outputDir, m_RCR, classesToSkip, m_PlatformProvider);

            EmscriptenCompiler.CleanupAndCreateEmscriptenDirs();
        }

        private void CompileBuild(BuildPostProcessArgs args)
        {
            var strippingInfo = ScriptableObject.CreateInstance<WebGLStrippingInfo>();
            strippingInfo.developmentBuild = m_DevelopmentPlayer;
            strippingInfo.builtCodePath = m_UseWasm ?
                Paths.Combine(m_StagingAreaData, kWasmLinkResult, kNativeFilename + kWasmCodeNativeExtension) :
                Paths.Combine(m_StagingAreaData, kAsmLinkResult, kNativeFilename + kAsmCodeNativeExtension);

            args.report.AddAppendix(strippingInfo);

            m_RCR = args.usedClassRegistry;
            var baselibLibraryDirectory = Paths.Combine(args.playerPackage, "Variations", m_DevelopmentPlayer ? "development" : "nondevelopment");
            m_PlatformProvider = new WebGlIl2CppPlatformProvider(BuildTarget.WebGL, args.stagingAreaData, kNativeFilename + kBitcodeExtension, args.report, baselibLibraryDirectory);
            m_PlatformProvider.Libs = GetModules();
            m_PlatformProvider.JsPre = new List<string>();
            m_PlatformProvider.JsLib = new List<string>();
            IL2CPPUtils.RunIl2Cpp(args.stagingAreaData, m_PlatformProvider, ModifyIl2CppOutputDirBeforeCompile, m_RCR);

            if (PlayerSettings.WebGL.useEmbeddedResources)
            {
                var embeddedResourcesDirectory = Paths.Combine(m_StagingAreaDataIl2CppData, "Resources");
                FileUtil.CreateOrCleanDirectory(embeddedResourcesDirectory);
                IL2CPPUtils.CopyEmbeddedResourceFiles(args.stagingAreaData, embeddedResourcesDirectory);
            }
            var metadataDirectory = Paths.Combine(m_StagingAreaDataIl2CppData, "Metadata");
            FileUtil.CreateOrCleanDirectory(metadataDirectory);
            IL2CPPUtils.CopyMetadataFiles(args.stagingAreaData, metadataDirectory);

            CopyMachineConfigFileFor(MonoInstallationFinder.MonoBleedingEdgeInstallation, "4.0");

            File.Copy(Paths.Combine(EmscriptenPaths.buildToolsDir, "data", "unity_default_resources"), Paths.Combine(m_StagingAreaDataResources, "unity_default_resources"));
        }

        private void CopyMachineConfigFileFor(string monoInstallation, string directoryVersionNumber)
        {
            var playerMachineConfigPath = Paths.Combine(m_StagingAreaDataManaged, "mono", directoryVersionNumber, "machine.config");
            Directory.CreateDirectory(Path.GetDirectoryName(playerMachineConfigPath));
            var monoEtcDirectory = MonoInstallationFinder.GetEtcDirectory(monoInstallation);
            File.Copy(monoEtcDirectory + "/" + directoryVersionNumber + "/machine.config", playerMachineConfigPath);
        }

        private void EmscriptenLink(BuildPostProcessArgs args, bool wasmBuild, string sourceFiles, string sourceFilesHash)
        {
            var removeDuplicateFunctions = !wasmBuild && !m_DevelopmentPlayer;
            var linkResult = wasmBuild ? kWasmLinkResult : kAsmLinkResult;
            var outputFolder = Paths.Combine(m_StagingAreaData, linkResult);
            var outputFilename = kNativeFilename + kFrameworkNativeExtension;
            FileUtil.CreateOrCleanDirectory(outputFolder);
            var outPath = Paths.Combine(EmscriptenPaths.GetShortPathName(outputFolder), outputFilename);
            var commandLine = ArgumentsForEmscripten(wasmBuild) + string.Format(" -o \"{0}\"", outPath) + sourceFiles;
            var fullHash = ComputeHashString(InternalEditorUtility.GetFullUnityVersion()) + ComputeHashString(commandLine) + sourceFilesHash + ComputeHashString(removeDuplicateFunctions);
            var cacheFolder = Paths.Combine(m_WebGLCache, linkResult + "_" + ComputeHashString(fullHash));

            if (Directory.Exists(cacheFolder))
            {
                FileUtil.CopyDirectoryRecursive(cacheFolder, outputFolder);
                return;
            }

            foreach (var directory in Directory.GetDirectories(m_WebGLCache, linkResult + "_*"))
                Directory.Delete(directory, true);

            var responseFilePath = Paths.Combine(EmscriptenPaths.dataPath, "..", "Temp", "emcc_arguments.resp");
            File.WriteAllText(responseFilePath, commandLine);
            var processStartInfo = new ProcessStartInfo(EmscriptenPaths.pythonExecutable)
            {
                // pass '-E' to ignore all PYTHON* environment variables, e.g. PYTHONPATH and PYTHONHOME, that might be set.
                Arguments = $"-E \"{EmscriptenPaths.emcc}\" @\"{responseFilePath}\"",
                WorkingDirectory = Path.GetFullPath(m_StagingAreaData),
                UseShellExecute = false,
                CreateNoWindow = true
            };
            EmccArguments.SetupDefaultEmscriptenEnvironment(processStartInfo);

            if (!ProgramUtils.StartProgramChecked(processStartInfo))
                throw new System.Exception("Error running Emscripten:\n" + processStartInfo.Arguments + "\n" + commandLine);

            if (removeDuplicateFunctions)
            {
                BuildStep(args, "Scripting", "Remove duplicate asm.js code");
                var NativePath = Paths.Combine(outputFolder, kNativeFilename);
                CodeAnalysisUtils.ReplaceDuplicates(NativePath + kAsmCodeNativeExtension, NativePath + kAsmSymbolsNativeExtension, NativePath + kAsmSymbolsStrippedExtension, 2);
            }

            FileUtil.CopyDirectoryRecursive(outputFolder, cacheFolder);
        }

        private void LinkBuild(BuildPostProcessArgs args)
        {
            var sourceFiles = "";
            var sourceFilesHash = "";

            foreach (var jsPre in GetJsprePlugins())
            {
                sourceFiles += string.Format(" --pre-js \"{0}\"", jsPre);
                sourceFilesHash += ComputeHashString(File.ReadAllBytes(jsPre));
            }

            foreach (var jsLib in GetJslibPlugins(args))
            {
                sourceFiles += string.Format(" --js-library \"{0}\"", jsLib);
                sourceFilesHash += ComputeHashString(File.ReadAllBytes(jsLib));
            }
            var bitcodePath = Paths.Combine(EmscriptenPaths.GetShortPathName(m_StagingAreaDataNative), kNativeFilename + kBitcodeExtension);
            sourceFiles += string.Format(" \"{0}\"", bitcodePath);
            sourceFilesHash += ComputeHashString(File.ReadAllBytes(bitcodePath));

            // keep this after bc, so linker won't ignore symbols in plugins
            foreach (var nativePlugin in GetNativePlugins(args))
            {
                sourceFiles += string.Format(" \"{0}\"", nativePlugin);
                sourceFilesHash += ComputeHashString(File.ReadAllBytes(nativePlugin));
            }

            BuildStep(args, "Scripting", "Compile " + (m_UseWasm ? "WebAssembly" : "asm.js") + " module");
            EmscriptenLink(args, m_UseWasm, sourceFiles, sourceFilesHash);
        }

        private void AssembleData(BuildPostProcessArgs args, string outputPath)
        {
            List<DataFile> dataFileList = new List<DataFile>();

            var buildFiles = new List<string>
            {
                Paths.Combine(m_StagingAreaDataResources, "unity_default_resources"),
                Paths.Combine(m_StagingAreaDataManaged, "mono", "4.0", "machine.config"),
            };
            buildFiles.AddRange(Directory.GetFiles(m_StagingAreaDataIl2CppData, "*.*", SearchOption.AllDirectories));
            buildFiles.AddRange(Directory.GetFiles(m_StagingAreaData).Where(f => !f.Contains("CAB-")));
            var subsystemsPath = Paths.Combine(m_StagingAreaData, "UnitySubsystems");
            if (Directory.Exists(subsystemsPath))
                buildFiles.AddRange(Directory.GetFiles(subsystemsPath, "*.*", SearchOption.AllDirectories));
            foreach (var path in buildFiles)
                dataFileList.Add(new DataFile(path, path.Substring(m_StagingAreaData.Length + 1)));
            var unityBuiltinExtraPath = Paths.Combine(m_StagingAreaDataResources, "unity_builtin_extra");
            if (File.Exists(unityBuiltinExtraPath))
                dataFileList.Add(new DataFile(unityBuiltinExtraPath, unityBuiltinExtraPath.Substring(m_StagingAreaData.Length + 1)));

            // The data file consisits of the header (which describes the stored files) and the body (which contains the actual files data).
            // The header consists of the data prefix (a null-terminated string which identifies the data file format), the total header size (uint), and file description records.
            // File description records are stored in the following form: offset in the data file (uint), file size (uint), file path length (uint), file path (string)

            byte[] dataPrefix = System.Text.Encoding.UTF8.GetBytes("UnityWebData1.0\0");
            long dataOffset = dataPrefix.Length + sizeof(uint);
            foreach (var file in dataFileList)
                dataOffset += 3 * sizeof(uint) + file.internalPath.Length;

            using (BinaryWriter writer = new BinaryWriter(File.Open(outputPath, FileMode.Create)))
            {
                writer.Write(dataPrefix);
                writer.Write((uint)dataOffset);
                foreach (var file in dataFileList)
                {
                    writer.Write((uint)dataOffset);
                    writer.Write((uint)file.length);
                    writer.Write((uint)file.internalPath.Length);
                    writer.Write(file.internalPath);
                    dataOffset += file.length;
                }
                foreach (var file in dataFileList)
                    writer.Write(File.ReadAllBytes(file.path));
            }
        }

        private void AssembleAsmCode(BuildPostProcessArgs args, string outputPath)
        {
            var asmCodePath = Paths.Combine(m_StagingAreaData, kAsmLinkResult, kNativeFilename) + kAsmCodeNativeExtension;
            File.Copy(asmCodePath, outputPath);
        }

        private void AssembleFramework(BuildPostProcessArgs args, bool useWasm, string outputPath)
        {
            var jsprePlugins = PluginImporter.GetImporters(args.target).Where(imp => imp.isNativePlugin && Path.GetExtension(imp.assetPath) == kJsprePluginExtension);
            var jspreBuilder = new System.Text.StringBuilder();
            foreach (PluginImporter jsprePlugin in jsprePlugins)
                jspreBuilder.Append(jspreBuilder.Length != 0 ? ";\n" : "").Append(File.ReadAllText(jsprePlugin.assetPath).Replace("\r\n", "\n").Trim().TrimEnd(';'));
            var linkResultPath = Paths.Combine(m_StagingAreaData, useWasm ? kWasmLinkResult : kAsmLinkResult);
            var frameworkCode = File.ReadAllText(Paths.Combine(linkResultPath, kNativeFilename + kFrameworkNativeExtension)).Replace("\"USER_JSPRE_PLACEHOLDER\"", jspreBuilder.ToString());
            File.WriteAllText(outputPath, PlayerSettings.WebGL.threadsSupport ?
                $"{frameworkCode}\nif (typeof importScripts == \"function\") {{\n{File.ReadAllText(Paths.Combine(linkResultPath, "pthread-main.js"))}\n}}\n" :
                $"function {kFrameworkFunctionName}(Module) {{\n{frameworkCode}\n}}\n"
            );
        }

        private bool AssembleBackground(BuildPostProcessArgs args, string outputPath)
        {
            var screenRect = new Rect(0, 0, PlayerSettings.defaultWebScreenWidth, PlayerSettings.defaultWebScreenHeight);
            var tex = PlayerSettingsSplashScreenEditor.GetSplashScreenActualBackgroundImage(screenRect);
            if (tex == null)
                return false;
            var rt = new RenderTexture(tex.width, tex.height, 0, RenderTextureFormat.ARGB32);
            Graphics.Blit(tex, rt);
            RenderTexture.active = rt;
            var readableTex = new Texture2D(tex.width, tex.height);
            var pixels = new Rect(0, 0, tex.width, tex.height);
            readableTex.ReadPixels(pixels, 0, 0);
            readableTex.Apply();
            File.WriteAllBytes(outputPath, readableTex.EncodeToJPG());
            return true;
        }

        private void AssembleDebugSymbols(BuildPostProcessArgs args, bool useWasm, string outputPath)
        {
            var symbolsPath = useWasm ?
                Paths.Combine(m_StagingAreaData, kWasmLinkResult, kNativeFilename + kWasmSymbolsNativeExtension) :
                Paths.Combine(m_StagingAreaData, kAsmLinkResult, kNativeFilename + kAsmSymbolsStrippedExtension);
            var symbols = CodeAnalysisUtils.ReadMinificationMap(symbolsPath).ToList();
            symbols.Sort((a, b) => System.String.Compare(a.Value, b.Value));
            using (StreamWriter file = new StreamWriter(outputPath))
            {
                file.Write("{");
                bool first = true;
                foreach (KeyValuePair<string, string> symbol in symbols)
                {
                    file.Write($"{(first ? "": ",")}\n \"{symbol.Key}\": \"{symbol.Value}\"");
                    first = false;
                }
                file.Write("\n}");
            }
            var processStartInfo = new ProcessStartInfo(EmscriptenPaths.nodeExecutable)
            {
                Arguments = $"Demangle.js \"{outputPath}\"",
                WorkingDirectory = EmscriptenPaths.buildToolsDir,
                UseShellExecute = false,
                CreateNoWindow = true
            };
            ProgramUtils.StartProgramChecked(processStartInfo);
        }

        private string PostProcessBuildFile(string path, string fileType, bool addBuildExtension)
        {
            // the function returns URL of the build file relative to the Build folder
            // note that all the build files are currently stored under the m_StagingAreaDataOutputBuild without subfolders
            if (Path.GetDirectoryName(path) != m_StagingAreaDataOutputBuild)
                throw new System.Exception("Invalid build file path: " + path);

            var outputPath = path;
            if (m_NameFilesAsHashes)
            {
                var hash = string.Join(string.Empty, System.Array.ConvertAll(m_Hash.ComputeHash(File.ReadAllBytes(path)), b => b.ToString("x2")));
                outputPath = Paths.Combine(Path.GetDirectoryName(path), hash + Path.GetExtension(path));
            }
            if (addBuildExtension)
                outputPath += GetBuildExtension();
            if (outputPath != path)
                File.Move(path, outputPath);

            m_BuildFiles.Add(outputPath, fileType);
            return Path.GetFileName(outputPath);
        }

        private void Preprocess(string inputPath, string outputPath, bool minifyOutput, string arguments)
        {
            var processStartInfo = new ProcessStartInfo(EmscriptenPaths.nodeExecutable)
            {
                Arguments = "Preprocess.js" + PreprocessorArgument("locals.inputPath", inputPath) + PreprocessorArgument("locals.outputPath", outputPath) + PreprocessorArgument("locals.minifyOutput", minifyOutput) + arguments,
                WorkingDirectory = EmscriptenPaths.buildToolsDir,
                UseShellExecute = false,
                CreateNoWindow = true
            };
            ProgramUtils.StartProgramChecked(processStartInfo);
        }

        private string GetBuildExtension()
        {
            return m_DevelopmentPlayer || PlayerSettings.WebGL.compressionFormat == WebGLCompressionFormat.Disabled ? "" :
                PlayerSettings.WebGL.decompressionFallback? kBuildExtension:
                PlayerSettings.WebGL.compressionFormat == WebGLCompressionFormat.Gzip ? kGzipExtension :
                PlayerSettings.WebGL.compressionFormat == WebGLCompressionFormat.Brotli ? kBrotliExtension :
                "";
        }

        private void AssembleOutput(BuildPostProcessArgs args)
        {
            var asmSuffix = m_UseWasm ? "" : ".asm";
            var loaderSuffix = asmSuffix + ".loader.js";
            var jsonSuffix = asmSuffix + ".json";
            var frameworkSuffix = asmSuffix + ".framework.js";
            var symbolsSuffix = asmSuffix + ".symbols.json";
            var codeSuffix = m_UseWasm ? ".wasm" : ".asm.js";
            var dataSuffix = ".data";
            var memorySuffix = asmSuffix + ".mem";

            FileUtil.CreateOrCleanDirectory(m_StagingAreaDataOutput);
            FileUtil.CopyDirectoryRecursiveForPostprocess(m_TemplatePath, m_StagingAreaDataOutput, true);
            FileUtil.UnityDirectoryRemoveReadonlyAttribute(m_StagingAreaDataOutput);
            File.Delete(Paths.Combine(m_StagingAreaDataOutput, "thumbnail.png"));

            Directory.CreateDirectory(m_StagingAreaDataOutputBuild);

            var frameworkFilename = "";
            var codeFilename = "";
            var memoryFilename = "";
            var symbolsFilename = "";

            var dataPath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + dataSuffix);
            AssembleData(args, dataPath);
            var dataFilename = PostProcessBuildFile(dataPath, kDataFileType, true);

            if (m_UseWasm)
            {
                var frameworkPath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + frameworkSuffix);
                AssembleFramework(args, true, frameworkPath);
                frameworkFilename = PostProcessBuildFile(frameworkPath, kWasmFrameworkFileType, true);

                var codePath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + codeSuffix);
                File.Copy(Paths.Combine(m_StagingAreaData, kWasmLinkResult, kNativeFilename + kWasmCodeNativeExtension), codePath);
                codeFilename = PostProcessBuildFile(codePath, kWasmCodeFileType, true);

                if (PlayerSettings.WebGL.threadsSupport)
                {
                    var memoryPath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + memorySuffix);
                    File.Copy(Paths.Combine(m_StagingAreaData, kWasmLinkResult, kNativeFilename) + kMemoryNativeExtension, memoryPath);
                    memoryFilename = PostProcessBuildFile(memoryPath, kWasmMemoryFileType, true);
                }

                if (!m_DevelopmentPlayer && PlayerSettings.WebGL.debugSymbols)
                {
                    var symbolsPath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + symbolsSuffix);
                    AssembleDebugSymbols(args, true, symbolsPath);
                    symbolsFilename = PostProcessBuildFile(symbolsPath, kWasmSymbolsFileType, true);
                }
            }
            else
            {
                var frameworkPath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + frameworkSuffix);
                AssembleFramework(args, false, frameworkPath);
                frameworkFilename = PostProcessBuildFile(frameworkPath, kAsmFrameworkFileType, true);

                var codePath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + codeSuffix);
                AssembleAsmCode(args, codePath);
                codeFilename = PostProcessBuildFile(codePath, kAsmCodeFileType, true);

                var memoryPath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + memorySuffix);
                File.Copy((Paths.Combine(m_StagingAreaData, kAsmLinkResult, kNativeFilename)) + kMemoryNativeExtension, memoryPath);
                memoryFilename = PostProcessBuildFile(memoryPath, kAsmMemoryFileType, true);

                if (!m_DevelopmentPlayer && PlayerSettings.WebGL.debugSymbols)
                {
                    var symbolsPath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + symbolsSuffix);
                    AssembleDebugSymbols(args, false, symbolsPath);
                    symbolsFilename = PostProcessBuildFile(symbolsPath, kAsmSymbolsFileType, true);
                }
            }

            var backgroundFilename = "";
            var backgroundPath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + kBackgroundExtension);
            if (AssembleBackground(args, backgroundPath))
                backgroundFilename = PostProcessBuildFile(backgroundPath, kBackgroundFileType, false);

            var useWebGL10 = false;
            var useWebGL20 = false;
            foreach (var api in PlayerSettings.GetGraphicsAPIs(BuildTarget.WebGL))
            {
                switch (api.ToString())
                {
                    case "OpenGLES2":
                        useWebGL10 = true;
                        break;
                    case "OpenGLES3":
                        useWebGL20 = true;
                        break;
                }
            }

            var PreprocessorArguments = "";

            PreprocessorArguments += PreprocessorArgument("COMPANY_NAME", PlayerSettings.companyName);
            PreprocessorArguments += PreprocessorArgument("PRODUCT_NAME", PlayerSettings.productName);
            PreprocessorArguments += PreprocessorArgument("PRODUCT_VERSION", PlayerSettings.bundleVersion);
            PreprocessorArguments += PreprocessorArgument("WIDTH", PlayerSettings.defaultWebScreenWidth);
            PreprocessorArguments += PreprocessorArgument("HEIGHT", PlayerSettings.defaultWebScreenHeight);
            PreprocessorArguments += PreprocessorArgument("SPLASH_SCREEN_STYLE", PlayerSettings.SplashScreen.unityLogoStyle == PlayerSettings.SplashScreen.UnityLogoStyle.DarkOnLight ? "Light" : "Dark");
            PreprocessorArguments += PreprocessorArgument("BACKGROUND_COLOR", "#" + ColorUtility.ToHtmlStringRGB(PlayerSettingsSplashScreenEditor.GetSplashScreenActualBackgroundColor()));

            PreprocessorArguments += PreprocessorArgument("DATA_FILENAME", dataFilename);
            PreprocessorArguments += PreprocessorArgument("FRAMEWORK_FILENAME", frameworkFilename);
            PreprocessorArguments += PreprocessorArgument("CODE_FILENAME", codeFilename);
            PreprocessorArguments += PreprocessorArgument("MEMORY_FILENAME", memoryFilename);
            PreprocessorArguments += PreprocessorArgument("SYMBOLS_FILENAME", symbolsFilename);
            PreprocessorArguments += PreprocessorArgument("BACKGROUND_FILENAME", backgroundFilename);

            PreprocessorArguments += PreprocessorArgument("USE_WASM", m_UseWasm);
            PreprocessorArguments += PreprocessorArgument("USE_THREADS", PlayerSettings.WebGL.threadsSupport);
            PreprocessorArguments += PreprocessorArgument("USE_WEBGL_1_0", useWebGL10);
            PreprocessorArguments += PreprocessorArgument("USE_WEBGL_2_0", useWebGL20);
            PreprocessorArguments += PreprocessorArgument("USE_DATA_CACHING", PlayerSettings.WebGL.dataCaching);
            PreprocessorArguments += PreprocessorArgument("DECOMPRESSION_FALLBACK", m_DevelopmentPlayer || !PlayerSettings.WebGL.decompressionFallback ? "" :
                PlayerSettings.WebGL.compressionFormat == WebGLCompressionFormat.Gzip ? "Gzip" : PlayerSettings.WebGL.compressionFormat == WebGLCompressionFormat.Brotli ? "Brotli" : "");
            PreprocessorArguments += PreprocessorArgument("TOTAL_MEMORY", m_TotalMemory);
            PreprocessorArguments += PreprocessorArgument("DEVELOPMENT_PLAYER", m_DevelopmentPlayer);
            PreprocessorArguments += PreprocessorArgument("UNITY_VERSION", Application.unityVersion);

            foreach (var customKey in PlayerSettings.templateCustomKeys)
                PreprocessorArguments += PreprocessorArgument(customKey, PlayerSettings.GetTemplateCustomValue(customKey));

            var loaderPath = Paths.Combine(m_StagingAreaDataOutputBuild, m_BuildName + loaderSuffix);
            Preprocess(Paths.Combine("UnityLoader", "UnityLoader.js"), loaderPath, !m_DevelopmentPlayer, PreprocessorArguments);
            var loaderFilename = PostProcessBuildFile(loaderPath, kUnityLoaderFileType, false);

            PreprocessorArguments += PreprocessorArgument("LOADER_FILENAME", loaderFilename);

            Regex preprocessedFilenameRegex = new Regex("\\.(html|php|css|js|json)$");
            foreach (var file in FileUtil.GetAllFilesRecursive(m_StagingAreaDataOutput))
            {
                if (preprocessedFilenameRegex.IsMatch(FileUtil.UnityGetFileName(file)) && FileUtil.UnityGetDirectoryName(file) != m_StagingAreaDataOutputBuild)
                    Preprocess(file, file, false, PreprocessorArguments);
            }
        }

        private static bool SetGzipComment(string path, string comment)
        {
            var data = File.ReadAllBytes(path);
            int commentOffset = 10, commentLength = 0;
            if (commentOffset > data.Length)
                return false;
            var flags = data[3];
            if ((flags & 0x04) != 0)
            {
                if (commentOffset + 2 > data.Length)
                    return false;
                commentOffset += 2 + data[commentOffset] + (data[commentOffset + 1] << 8);
                if (commentOffset > data.Length)
                    return false;
            }
            if ((flags & 0x08) != 0)
            {
                while (commentOffset < data.Length && data[commentOffset] != 0)
                    commentOffset++;
                if (commentOffset + 1 > data.Length)
                    return false;
                commentOffset++;
            }
            if ((flags & 0x10) != 0)
            {
                while (commentOffset + commentLength < data.Length && data[commentOffset + commentLength] != 0)
                    commentLength++;
                if (commentOffset + commentLength + 1 > data.Length)
                    return false;
                commentLength++;
            }
            using (BinaryWriter writer = new BinaryWriter(File.Open(path, FileMode.Create)))
            {
                data[3] |= 0x10;
                writer.Write(data, 0, commentOffset);
                writer.Write(System.Text.Encoding.UTF8.GetBytes(comment + '\0'));
                writer.Write(data, commentOffset + commentLength, data.Length - commentOffset - commentLength);
            }
            return true;
        }

        public static void CompressAndMarkGzip(string path)
        {
            var compressedPath = path + kCompressedExtension;
            var processPath = Paths.Combine(EditorApplication.applicationContentsPath, "Tools", Application.platform == RuntimePlatform.WindowsEditor ? "7z.exe" : "7za");
            var processArguments = string.Format("a -tgzip \"{0}\" \"{1}\"", compressedPath, path);
            var startInfo = new ProcessStartInfo(processPath)
            {
                Arguments = processArguments,
                WorkingDirectory = Directory.GetCurrentDirectory(),
                UseShellExecute = false,
                CreateNoWindow = true,
            };
            ProgramUtils.StartProgramChecked(startInfo);
            if (!SetGzipComment(compressedPath, kCompressionMarkerGzip))
                throw new System.Exception("Invalid gzip archive format: " + compressedPath);
            if (File.Exists(path)) // Linux gzip already deletes source files.
                File.Delete(path);
            File.Move(compressedPath, path);
        }

        public static void CompressAndMarkBrotli(string path)
        {
            var compressedPath = path + kCompressedExtension;
            var brotliWrapper = Paths.Combine(EmscriptenPaths.buildToolsDir, "Brotli", "python", "bro.py");
            var brotliBinary = "Brotli-0.4.0-py2.7-" + (Application.platform == RuntimePlatform.WindowsEditor ? "win-amd64" :
                Application.platform == RuntimePlatform.LinuxEditor ? "linux-x86_64" : "macosx-10.10-x86_64") + ".egg";
            var startInfo = new ProcessStartInfo(EmscriptenPaths.pythonExecutable)
            {
                Arguments = string.Format("\"{0}\" -o \"{1}\" -i \"{2}\" --comment \"{3}\"", brotliWrapper, compressedPath, path, kCompressionMarkerBrotli),
                WorkingDirectory = Directory.GetCurrentDirectory(),
                UseShellExecute = false,
                CreateNoWindow = true,
            };
            startInfo.EnvironmentVariables["PYTHONPATH"] = Paths.Combine(EmscriptenPaths.buildToolsDir, "Brotli", "dist", brotliBinary);
            ProgramUtils.StartProgramChecked(startInfo);
            File.Delete(path);
            File.Move(compressedPath, path);
        }

        private void CompressBuild(BuildPostProcessArgs args)
        {
            var filenames = Directory.GetFiles(m_StagingAreaDataOutputBuild, "*" + GetBuildExtension(), SearchOption.TopDirectoryOnly);
            foreach (string path in filenames)
                if (m_BuildFiles.ContainsKey(path))
                    switch (PlayerSettings.WebGL.compressionFormat)
                    {
                        case WebGLCompressionFormat.Brotli:
                            CompressAndMarkBrotli(path);
                            break;
                        case WebGLCompressionFormat.Gzip:
                            CompressAndMarkGzip(path);
                            break;
                    }
        }

        private void MoveOutputToInstallPath(BuildPostProcessArgs args)
        {
            foreach (string path in Directory.GetDirectories(m_StagingAreaDataOutput, "*", SearchOption.AllDirectories))
                FileUtil.CreateOrCleanDirectory(Paths.Combine(args.installPath, path.Substring(m_StagingAreaDataOutput.Length + 1)));
            foreach (string path in Directory.GetFiles(m_StagingAreaDataOutput, "*", SearchOption.AllDirectories))
            {
                var finalPath = Paths.Combine(args.installPath, path.Substring(m_StagingAreaDataOutput.Length + 1));
                File.Copy(path, finalPath, true);
                args.report.RecordFileAdded(finalPath, m_BuildFiles.ContainsKey(path) ? m_BuildFiles[path] : kTemplateFileType);
            }
        }

        public override void PostProcess(BuildPostProcessArgs args)
        {
            if (PlayerSettings.WebGL.linkerTarget != WebGLLinkerTarget.Wasm)
            {
                Debug.LogWarning("PlayerSettings.WebGL.linkerTarget should be set to WebGLLinkerTarget.Wasm in your script.");
                if (PlayerSettings.WebGL.memorySize > 32)
                {
                    Debug.LogWarning($"PlayerSettings.WebGL.memorySize is set to {PlayerSettings.WebGL.memorySize} but it will not have any effect on WebAssembly builds.");
                }
            }

            m_DevelopmentPlayer = (args.options & BuildOptions.Development) != 0;
            m_UseWasm = PlayerSettings.WebGL.linkerTarget != WebGLLinkerTarget.Asm;
            m_NameFilesAsHashes = PlayerSettings.WebGL.nameFilesAsHashes && !m_DevelopmentPlayer;

            if (m_DevelopmentPlayer && (args.options & BuildOptions.AllowDebugging) != 0)
                Debug.LogWarning("Script debugging is not supported on WebGL platform, BuildOptions.AllowDebugging option will be ignored.");

            m_StagingAreaData = Path.GetFullPath(args.stagingAreaData);
            m_StagingAreaDataOutput = Paths.Combine(m_StagingAreaData, kOutputFolder);
            m_StagingAreaDataOutputBuild = Paths.Combine(m_StagingAreaDataOutput, kBuildFolder);
            m_StagingAreaDataNative = Paths.Combine(m_StagingAreaData, kNativeFolder);
            m_StagingAreaDataManaged = Paths.Combine(m_StagingAreaData, kManagedFolder);
            m_StagingAreaDataResources = Paths.Combine(m_StagingAreaData, kResourcesFolder);
            m_StagingAreaDataIl2CppData = Paths.Combine(m_StagingAreaData, kIl2CppDataFolder);
            m_BuildName = new DirectoryInfo(args.installPath).Name;
            // round heap size to next multiple of 16mb
            m_TotalMemory = ((int)System.Math.Truncate((PlayerSettings.WebGL.memorySize + 15.0) / 16.0) * 16 * 1024 * 1024).ToString();
            string[] templateElements = PlayerSettings.WebGL.template.Split(':');
            m_TemplatePath = Paths.Combine(templateElements[0].Equals("PROJECT") ? EmscriptenPaths.dataPath : EmscriptenPaths.buildToolsDir, "WebGLTemplates", templateElements[1]);
            m_BuildFiles = new Dictionary<string, string>();

            m_WebGLCache = Paths.Combine(EmscriptenPaths.GetShortPathName(Path.GetFullPath("Library")), kWebGLCache);
            if (!Directory.Exists(m_WebGLCache))
                Directory.CreateDirectory(m_WebGLCache);

            // Windows file system won't let us overwrite files if we are hosting them from a previous
            // Build & Run call. So kill any previous web server we spawned.
            if (Application.platform == RuntimePlatform.WindowsEditor)
                HttpServerEditorWrapper.Kill();

            m_Progress.Reset(1);
            Directory.CreateDirectory(args.installPath);
            CheckBuildPrerequisites();

            BuildStep(args, "Scripting", "Convert and compile scripting files");
            CompileBuild(args);
            LinkBuild(args);

            BuildStep(args, "Files", "Packaging files");
            AssembleOutput(args);

            if (!m_DevelopmentPlayer && PlayerSettings.WebGL.compressionFormat != WebGLCompressionFormat.Disabled)
            {
                BuildStep(args, "Compress", "Compressing build results");
                CompressBuild(args);
            }

            BuildStep(args, "Files", "Copying files to the final destination");
            MoveOutputToInstallPath(args);
            PostprocessBuildPlayer.InstallStreamingAssets(args.installPath, args.report);

            WebsockifyEditorWrapper.CreateIfNeeded();
        }
    }
}
