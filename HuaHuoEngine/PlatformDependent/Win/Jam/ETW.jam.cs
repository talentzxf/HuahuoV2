using System;
using System.Linq;
using NiceIO;
using Bee.Core;
using Bee.DotNet;
using Bee.NativeProgramSupport;
using Bee.Toolchain.VisualStudio;
using Bee.Toolchain.Windows;
using Unity.BuildTools;
using Projects.Jam;

namespace PlatformDependent.Win.Etw
{
    static class UnityEtwProvider
    {
        static readonly NPath MessageManifest = new NPath("PlatformDependent/Win/etw/UnityETWProvider.man");

        static NPath GeneratedFilePrefix => new NPath($"artifacts/{nameof(UnityEtwProvider)}/{MessageManifest.FileNameWithoutExtension}.gen");

        public static NPath GeneratedHeaderFilePath => $"{GeneratedFilePrefix}.h";

        public static Lazy<BuiltNativeProgram> EtwProviderPlugin { get; } = new Lazy<BuiltNativeProgram>(() =>
        {
            var toolchain = ToolChains.ForWindows(Architecture.x64);

            // The resourceFile and header are created by running the ETW message compiler.
            Backend.Current.AddAction("VisualStudio_EtwMessageManifestCompiler",
                targetFiles: new NPath[]
                {
                    $"{GeneratedFilePrefix}.rc",
                    $"{GeneratedFilePrefix}.h"
                },
                inputs: new[]
                {
                    MessageManifest
                },
                executableStringFor: toolchain.Sdk.ToolPath("mc.exe").InQuotes(SlashMode.Native),
                commandLineArguments: new[]
                {
                    "-um",
                    "-z",
                    GeneratedFilePrefix.InQuotes(),
                    MessageManifest.InQuotes()
                });

            var program = new NativeProgram(nameof(UnityEtwProvider))
            {
                Sources = {$"{GeneratedFilePrefix}.rc"}
            };
            program.DynamicLinkerSettingsForMsvc().Add(l => l.WithoutEntryPoint());

            var result = program.SetupSpecificConfiguration(new NativeProgramConfiguration(CodeGen.Release, toolchain, false), toolchain.DynamicLibraryFormat);

            // We also need the original manifest file in order to register the lib, and
            // the registration helper scripts, and the WPR trace profiles
            result = result.WithDeployables(new IDeployable[]
            {
                new DeployableFile(MessageManifest),
                new DeployableFile(new NPath("PlatformDependent/Win/etw/InstallProvider.ps1")),
                new DeployableFile(new NPath("PlatformDependent/Win/etw/UninstallProvider.ps1"))
            }.Concat(
                    new NPath("PlatformDependent/Win/etw")
                        .Files(new[] { "wprp" }, true)
                        .Select(f => new DeployableFile(f))
                    ).ToArray());

            return result;
        });
    }
}
