using System;
using System.Collections.Generic;
using System.Linq;
using Bee.Core;
using Bee.Core.Stevedore;
using Bee.CSharpSupport;
using Bee.DotNet;
using Bee.NativeProgramSupport;
using Bee.Stevedore;
using Bee.Toolchain.Emscripten;
using Bee.Toolchain.GNU;
using Bee.UnityTools;
using JamSharp.Runtime;
using Modules;
using NiceIO;
using Projects.Jam;
using Projects.Jam.Rules;
using Unity.BuildSystem;
using Unity.BuildSystem.Common;
using Unity.BuildSystem.WebGLSupport;
using Unity.BuildSystem.V2;

namespace PlatformDependent.WebGL.Jam
{
    [Help(
        "WebGLPlayer",
        "WebGL Player",
        category: Help.Category.OtherPlayers,
        configs: Help.Configs.None,
        notes: new[] {"Consult jam --help WebGLPlayer for a list of all variations"})]

    public class UnityWebGLPlayerConfiguration : UnityPlayerConfiguration
    {
        public bool IsThreaded { get; }
        public OptimizationLevel? Optimization { get; }

        public UnityWebGLPlayerConfiguration(
            CodeGen codeGen,
            ToolChain toolChain,
            bool lump,
            ScriptingBackend scriptingBackend,
            bool developmentPlayer,
            bool enableUnitTests,
            bool isThreaded,
            OptimizationLevel? optimization)
            : base(codeGen, toolChain, lump, scriptingBackend, developmentPlayer: developmentPlayer, enableUnitTests: enableUnitTests)
        {
            IsThreaded = isThreaded;
            Optimization = optimization;
        }

        protected override string IdentifierFor(string toolchainIdentifier)
        {
            var ret = base.IdentifierFor(toolchainIdentifier);
            if (!IsThreaded)
                ret += "_nothreads";
            if (Optimization != null)
                ret += "_opt" + Optimization.ToString().ToLowerInvariant();
            return ret;
        }

        public override bool Equals(object obj)
        {
            if (obj is UnityWebGLPlayerConfiguration other)
                return base.Equals(other) && IsThreaded == other.IsThreaded && Optimization == other.Optimization;
            return false;
        }

        public override int GetHashCode()
        {
            var result = (IsThreaded ? 1 : 0) ^ (Optimization == null ? 0 : 2 * (1 + (int)Optimization));
            // hash code for the subclass is shifted in order to avoid unnecessary collisions when performing xor with the hash of the base class
            return base.GetHashCode() ^ (result << 16);
        }
    }

    public class WebGLSupport : PlatformSupport
    {
        private readonly EmscriptenToolchain _emscriptenToolchain = new EmscriptenToolchain(new UnityEmscriptenSdk(EmscriptenArchitecture.AsmJs));
        protected override string PlatformName => "WebGL";

        protected override string LegacyTargetName { get; } = "WebGLSupport";
        protected override string[] DeveloperTargetAliases { get; } = {"WebGLSupportAll"};
        protected override string VariationPrefix { get; } = "WebGLPlayer";
        protected override bool SupportPerformanceTests => false;
        public override string[] CommandLinesOfInterest { get; } = {"WebGL"};
        public override string EditorExtensionBuildTarget => "WebGLEditorExtensions";
        protected string[] ExtraExportedRuntimeMethods = new string[]
        {
            "addRunDependency",
            "removeRunDependency",
            "FS_createPath",
            "FS_createDataFile",
            "ccall",
            "cwrap",
            "stackTrace"
        };

        public static StevedoreArtifact EmscriptenSdkMac { get; } = StevedoreArtifact.FromLocalFile("emscripten-sdk-mac", "External/Emscripten/EmscriptenSdk_Mac/builds.7z");

        //Linux actually uses the same sdk as mac.
        public static StevedoreArtifact EmscriptenSdkLinux { get; } = StevedoreArtifact.FromLocalFile("emscripten-sdk-mac", "External/Emscripten/EmscriptenSdk_Mac/builds.7z");

        public static BuildZipArtifact PhysXSDK { get; } = new BuildZipArtifact("PlatformDependent/WebGL/External/PhysX3/builds.7z");
        private StevedoreArtifact Brotli { get; } = StevedoreArtifact.FromLocalFile("brotli", "External/Compression/Brotli/Brotli/builds.zip");

        protected override IEnumerable<UnityPlayerConfiguration> ProvideVariations()
        {
            yield return MakeConfiguration(developmentPlayer: true, enableUnitTests: false, isThreaded: false, optimization: null);
            yield return MakeConfiguration(developmentPlayer: false, enableUnitTests: false, isThreaded: false, optimization: null);
            yield return MakeConfiguration(developmentPlayer: false, enableUnitTests: false, isThreaded: false, optimization: OptimizationLevel.Size);
            yield return MakeConfiguration(developmentPlayer: true, enableUnitTests: true, isThreaded: false, optimization: null);

            yield return MakeConfiguration(developmentPlayer: true, enableUnitTests: false, isThreaded: true, optimization: null);
            yield return MakeConfiguration(developmentPlayer: false, enableUnitTests: false, isThreaded: true, optimization: null);
            yield return MakeConfiguration(developmentPlayer: false, enableUnitTests: false, isThreaded: true, optimization: OptimizationLevel.Size);
            yield return MakeConfiguration(developmentPlayer: true, enableUnitTests: true, isThreaded: true, optimization: null);
        }

        private UnityPlayerConfiguration MakeConfiguration(bool developmentPlayer, bool enableUnitTests, bool isThreaded, OptimizationLevel? optimization)
        {
            return new UnityWebGLPlayerConfiguration(CodeGen.Release, _emscriptenToolchain, true, ScriptingBackend.IL2CPP,
                developmentPlayer: developmentPlayer, enableUnitTests: enableUnitTests, isThreaded: isThreaded, optimization: optimization);
        }

        protected override string EditorExtensionSourceDirectory => "PlatformDependent/WebGL/Extensions/Unity.WebGL.extensions";
        protected override string EditorExtensionAssembly => "UnityEditor.WebGL.Extensions.dll";
        protected override string[] PlatformDependentFolders => new[] {"WebGL/Source"};

        protected override IEnumerable<string> Defines => Configuration.GlobalDefines.GetCppPlatformDefines("PLATFORM_WEBGL");

        protected override string PlayerBinaryName { get; } = "WebGLPlayer";
        protected override IEnumerable<string> CsDefines => Configuration.GlobalDefines.GetCsharpPlatformDefines("PLATFORM_WEBGL");
        protected override bool MatchVariationWithLegacyCommandLineArgs(UnityPlayerConfiguration variation)
        {
            var isDevel = ConvertedJamFile.Vars.DEVELOPMENT_PLAYER == "1";
            var unittests = ConvertedJamFile.Vars.ENABLE_UNIT_TESTS;

            return (variation.DevelopmentPlayer == isDevel ||
                ConvertedJamFile.Vars.DEVELOPMENT_PLAYER.IsEmpty) &&
                (!unittests.HasValue || variation.EnableUnitTests == unittests);
        }

        protected override string[] ModuleBlacklist { get; } = {"ClusterRenderer", "ClusterInput", "VirtualTexturing"};
        protected override ModuleBuildMode ModuleBuildMode => ModuleBuildMode.StaticLibrary;
        protected override NPath DefaultPrefixPch => null;

        protected override NPath BuildVariationDirFor(UnityPlayerConfiguration config)
        {
            var dirName = $"{BuildFolder}/Variations/{(config.EnableUnitTests ? "testing" : config.DevelopmentPlayer ? "development" : "nondevelopment")}";
            if (config is UnityWebGLPlayerConfiguration webGLConfig)
            {
                if (webGLConfig.IsThreaded)
                    dirName += "_mt";
                if (webGLConfig.Optimization != null)
                    dirName += "_opt" + webGLConfig.Optimization.ToString().ToLowerInvariant();
            }
            return dirName;
        }

        public static WebGLSupport Instance { get; private set; }

        public WebGLSupport()
        {
            Instance = this;
        }

        public override void Setup()
        {
            base.Setup();

            // everything below is only needed when doing "actual build"
            if (!ActualBuildRequested)
                return;

            var simpleWebServer = new CSharpProgram
            {
                Path = BuildFolder.Combine("BuildTools/SimpleWebServer.exe"),
                Sources = { "PlatformDependent/WebGL/SimpleWebServer" },
                Framework = {Framework.Framework46},
            };
            CommonVariationDependencies.DependOn(simpleWebServer.SetupDefault().Path);

            CopyDir("PlatformDependent/WebGL/prejs", BuildTools.Combine("prejs"));
            CopyDir("External/uglify-js", BuildFolder.Combine("BuildTools/uglify-js"));
            CopyDir("External/acorn", BuildFolder.Combine("BuildTools/acorn"));
            CopyDir("PlatformDependent/WebGL/WebGLTemplates", BuildTools.Combine("WebGLTemplates"));
            CopyDir("External/websockify", BuildTools.Combine("websockify"));

            if (string.IsNullOrEmpty(Environment.GetEnvironmentVariable("EMSDK")))
            {
                CommonVariationDependencies.DependOn(UnityEmscriptenSdk.Emscripten.DeployTo(BuildTools.Combine("Emscripten")).GetFileList());
                CommonVariationDependencies.DependOn(UnityEmscriptenSdk.EmscriptenSdkWin.DeployTo(BuildTools.Combine("Emscripten_Win")).GetFileList());
                CommonVariationDependencies.DependOn(UnityEmscriptenSdk.EmscriptenFastcompWin.DeployTo(BuildTools.Combine("Emscripten_FastComp_Win")).GetFileList());

                CommonVariationDependencies.DependOn(EmscriptenSdkMac.UnpackToUnusualLocation(BuildTools.Combine("Emscripten_Mac")));
                CommonVariationDependencies.DependOn(UnityEmscriptenSdk.EmscriptenFastcompMac.DeployTo(BuildTools.Combine("Emscripten_FastComp_Mac")).GetFileList());

                // Linux and Mac use the same emscripten sdk
                CommonVariationDependencies.DependOn(EmscriptenSdkLinux.UnpackToUnusualLocation(BuildTools.Combine("Emscripten_Linux")));
                CommonVariationDependencies.DependOn(UnityEmscriptenSdk.EmscriptenFastcompLinux.DeployTo(BuildTools.Combine("Emscripten_FastComp_Linux")).GetFileList());
            }

            CommonVariationDependencies.DependOn(Brotli.UnpackToUnusualLocation(BuildTools.Combine("Brotli")));

            foreach (var file in new NPath("PlatformDependent/WebGL/js").Files())
                CopyFile(file, BuildTools.Combine($"lib/{file.FileName}"));

            CopyFile(RuntimeResources.Path.Combine("resources_webgl"), BuildTools.Combine("data/unity_default_resources"));
            CopyFile("PlatformDependent/WebGL/emscripten.config", BuildTools.Combine("emscripten.config"));
            CopyFile("PlatformDependent/WebGL/ExceptionLogger.js", BuildTools.Combine("ExceptionLogger.js"));
            CopyFile("PlatformDependent/WebGL/DynamicJslibLoader.js", BuildTools.Combine("DynamicJslibLoader.js"));
            CopyFile("PlatformDependent/WebGL/Demangle.js", BuildTools.Combine("Demangle.js"));
            CopyFile("PlatformDependent/WebGL/Preprocess.js", BuildTools.Combine("Preprocess.js"));
            CopyFile("PlatformDependent/WebGL/UserJsprePlaceholder.js", BuildTools.Combine("UserJsprePlaceholder.js"));

            foreach (var file in new NPath("PlatformDependent/WebGL/UnityLoader").Files())
                CopyFile(file, BuildTools.Combine($"UnityLoader/{file.FileName}"));

            var brotlifiles = new NPath[]
            {
                "decompress.js",
                "dec/bit_reader.js",
                "dec/context.js",
                "dec/decode.js",
                "dec/dictionary.js",
                "dec/dictionary.bin.js",
                "dec/dictionary-browser.js",
                "dec/huffman.js",
                "dec/prefix.js",
                "dec/streams.js",
                "dec/transform.js",
                "node_modules/base64-js/index.js"
            };
            CommonVariationDependencies.DependOn(new BrowserifyJsTool().SetupInvocation(BuildTools.Combine("UnityLoader/Brotli.js"), "External/Compression/Brotli/brotli.js-1.3.1", brotlifiles));

            var gzipfiles = new NPath[]
            {
                "inflate.js",
                "utils/common.js",
                "utils/strings.js",
                "zlib/inflate.js",
                "zlib/constants.js",
                "zlib/messages.js",
                "zlib/zstream.js",
                "zlib/gzheader.js",
                "zlib/adler32.js",
                "zlib/crc32.js",
                "zlib/inffast.js",
                "zlib/inftrees.js"
            };
            CommonVariationDependencies.DependOn(new BrowserifyJsTool().SetupInvocation(BuildTools.Combine("UnityLoader/Gzip.js"), "External/Compression/gzip/pako-1.0.4/lib", gzipfiles));
        }

        NPath BuildTools => BuildFolder.Combine("BuildTools");

        // build code might get executed for non-"actual" build too (e.g. project files, reference assemblies etc.)
        static bool ActualBuildRequested => ConvertedJamFile.Vars.JAM_COMMAND_LINE_TARGETS.Any(s => s.StartsWith("WebGL"));

        private void CopyFile(NPath fromFile, NPath toFile)
        {
            CommonVariationDependencies.DependOn(Copy.Setup(fromFile, toFile));
        }

        private void CopyDir(NPath fromDir, NPath targetDir)
        {
            FastCopyDirectory.Instance.DeleteDestinationAndCopy(CommonVariationDependencies.ToString(), targetDir.ToString(), fromDir);
        }

        protected override void SetupModule(ModuleBase module)
        {
            base.SetupModule(module);
            var coreModule = module as CoreModule;
            if (coreModule != null)
            {
                var cpp = coreModule.Cpp;
                cpp.Add(RuntimeFiles.SourcesNoPCH);
                cpp.Add(RuntimeFiles.SourcesGfxOpenGLES);
                cpp.Add(c => c is UnityWebGLPlayerConfiguration webGLConfig && webGLConfig.IsThreaded, RuntimeFiles.SourcesGfxThreaded);
                cpp.Add("External/HashFunctions/md5.cpp");
                cpp.Add("Runtime/VirtualFileSystem/LocalFileSystemPosix.cpp");
                cpp.Add("Runtime/Input/SimulateInputEvents.cpp");
                cpp.Add(RuntimeFiles.SourcesGfxNull);
                cpp.Add("PlatformDependent/WebGL/Source/gles/gl.cpp");
                cpp.Add("PlatformDependent/WebGL/Source/Utilities/PlatformLogAssert.cpp");

                // Modules on in the WebGL build bundle in all static libraries and this is the primary way how we get libraries into WebGL (see SetupVariation)
                // However, it would be wasteful to bundle Baselib into all modules, so instead we only bundle it into the CoreModule.
                External.Baselib.AddDependencyToNativeProgram(NativeProgramForModule(coreModule), PlatformExternalDependentFolder, includesOnly: false);
            }
        }

        protected override void PostProcessNativeProgram(NativeProgram app, NativeProgram[] allPrograms)
        {
            base.PostProcessNativeProgram(app, allPrograms);

            PlayerProgram.Libraries.Add(new NPath("PlatformDependent/WebGL/prejs").Files("*.js").Select(l => new PreJsLibrary(l)));
            PlayerProgram.Libraries.Add(new NPath("PlatformDependent/WebGL/js").Files("*.js").Select(l => new JavascriptLibrary(l)));

            foreach (var app2 in allPrograms)
            {
                /* Emscripten has a bug where -Werror=return-type is interpreted as
                 * -Werror=return-type-c-linkage, which breaks physics code, and
                 * -Werror=incompatible-pointer-types as -Werror=incompatible-pointer-types-discards-qualifiers
                 * So, we filter it out here, and don't call the base class PostProcessNativeProgram
                 *
                 */

                app2.CompilerSettingsForEmscripten().Add(s =>
                    s.WithWarningPolicies(WarningSettings_GccClang.Policies(s.Language, Compiler.Clang)
                        .Where(p => !(p.Warning.Contains("return-type") || p.Warning.Contains("incompatible-pointer-types"))).ToArray()));

                app2.CompilerSettingsForEmscripten().Add(c => c is UnityWebGLPlayerConfiguration webGLConfig && webGLConfig.Optimization == OptimizationLevel.None, c => c.WithOptimizationLevel(OptimizationLevel.None));
                app2.CompilerSettingsForEmscripten().Add(c => c is UnityWebGLPlayerConfiguration webGLConfig && webGLConfig.Optimization == OptimizationLevel.Size, c => c.WithOptimizationLevel(OptimizationLevel.Size));
                app2.CompilerSettingsForEmscripten().Add(c => c is UnityWebGLPlayerConfiguration webGLConfig && webGLConfig.Optimization == OptimizationLevel.Speed, c => c.WithOptimizationLevel(OptimizationLevel.Speed));
                app2.CompilerSettingsForEmscripten().Add(c => c is UnityWebGLPlayerConfiguration webGLConfig && webGLConfig.Optimization == OptimizationLevel.SpeedMax, c => c.WithOptimizationLevel(OptimizationLevel.SpeedMax));

                app2.CompilerSettingsForEmscripten().Add(c => c.WithOptimizationLevel(OptimizationLevel.None), "Modules/Physics/PhysicsQuery.cpp");
                // disable optimizations on Shadows.cpp: there is a code-gen issue causing an integer overflow trap when running 361-ReflectionProbesRealtime gfx test
                app2.CompilerSettingsForEmscripten().Add(c => c.WithOptimizationLevel(OptimizationLevel.None), "Runtime/Camera/Shadows.cpp");
                // disable optimizations on PerformanceReportingRenderingInfo.cpp to workaround an integer overflow wasm trap (FLT_MAX * 1000.f is assigned to an integer)
                app2.CompilerSettingsForEmscripten().Add(c => c.WithOptimizationLevel(OptimizationLevel.None), "Modules/PerformanceReporting/PerformanceReportingRenderingInfo.cpp");
                // disable optimizations on AnimationClip.cpp to workqround an invalid conversion trap
                app2.CompilerSettingsForEmscripten().Add(c => c.WithOptimizationLevel(OptimizationLevel.None), "Modules/Animation/AnimationClip.cpp");

                app2.Defines.Add(c => ((UnityPlayerConfiguration)c).EnableUnitTests, "UNITTEST_FORCE_NO_POSIX", "UNITTEST_FORCE_NO_EXCEPTIONS");
                app2.Defines.Add(c => c is UnityWebGLPlayerConfiguration webGLConfig && webGLConfig.IsThreaded, "PLATFORM_SUPPORTS_THREADS=1", "UNITY_POSIX=1", "UNITY_ATOMIC_FORCE_LOCKFREE_IMPLEMENTATION=0", "UNITY_PLATFORM_THREAD_TO_CORE_MAPPING=1");

                // TODO: enable clang atomics (they don't work properly yet)
                // app2.Defines.Add(c => c is UnityWebGLPlayerConfiguration webGLConfig && webGLConfig.IsThreaded, "PLATFORM_SUPPORTS_THREADS=1", "UNITY_POSIX=1", "UNITY_ATOMIC_USE_CLANG_ATOMICS=1", "UNITY_ATOMIC_FORCE_LOCKFREE_IMPLEMENTATION=0");

                app2.IncludeDirectories.Add("PlatformDependent/WebGL/Source",
                    "PlatformDependent/WebGL/Source/Threads",
                    "External/il2cpp/builds/external/bdwgc/include");

                // The lumping blacklist is already applied to the PlayerProgram, but because we build not in source mode
                // there might still be some value in applying it to all the other NativePrograms too, here.
                app2.NonLumpableFiles.Add(NonLumpableFiles.LumpingBlacklist.ToNPaths());

                if (app2 != app)
                    app2.OutputName.Set($"WebGLSupport_{app2.Name}Module_Dynamic");
            }

            // WebGL build puts stuff into separate modules, with nothing directly under "main" app; but for IDE project files
            // we want all source files to appear under a single "main" target
            if (ProjectFiles.IsRequested)
            {
                foreach (var app2 in allPrograms)
                {
                    if (app2 != app)
                        app2.Sources.CopyTo(app.Sources);
                }
            }
        }

        protected override bool UseRTTIFor(UnityPlayerConfiguration arg) => false;

        protected override void SetupVariation(UnityPlayerConfiguration variation)
        {
            var variationVirtualTarget = VariationVirtualTargetFor(variation);

            foreach (var moduleNativeProgram in ModuleNativePrograms)
            {
                // on webgl we bundle static library dependencies. the correct thing to do here is probably
                // to not do that ,but to track the dependencies and add them to the final compilation.
                moduleNativeProgram.StaticLinkerSettings().Add(c => c == variation, l => l.WithBundledStaticLibraryDependencies(true));
                var targetDirectory = BuildFolder.Combine($"BuildTools/lib/{ModulesDirectoryNameFor(variation)}");

                var specificModuleLibrary = (StaticLibrary)moduleNativeProgram.SetupSpecificConfiguration(variation,
                    variation.ToolChain.StaticLibraryFormat)
                    .DeployTo(targetDirectory);
                variationVirtualTarget.DependOn(specificModuleLibrary.Path);
                PlayerProgram.Libraries.Add(c => c == variation, specificModuleLibrary);
            }

            variationVirtualTarget.DependOn(Il2CppBaselibBuild.ForUnity(variation).DeployTo(BuildVariationDirFor(variation)).Path);
        }

        private string ModulesDirectoryNameFor(UnityPlayerConfiguration variation)
        {
            var dirName = $"modules{(variation.EnableUnitTests ? "_testing" : variation.DevelopmentPlayer ? "_development" : "")}";
            if (variation is UnityWebGLPlayerConfiguration webGLConfig)
            {
                if (webGLConfig.IsThreaded)
                    dirName += "_mt";
                if (webGLConfig.Optimization != null)
                    dirName += "_opt" + webGLConfig.Optimization.ToString().ToLowerInvariant();
            }
            return dirName;
        }
    }
}
