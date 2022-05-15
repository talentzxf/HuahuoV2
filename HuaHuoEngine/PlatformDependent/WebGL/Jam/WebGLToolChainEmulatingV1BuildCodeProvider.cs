using System.Diagnostics.CodeAnalysis;
using Bee.Core;
using Bee.NativeProgramSupport;
using Bee.Toolchain.Emscripten;
using Unity.BuildSystem;
using Unity.BuildSystem.WebGLSupport;

[SuppressMessage("ReSharper", "UnusedMember.Global")] // used through reflection
class WebGLToolChainEmulatingV1BuildCodeProvider : IToolChainEmulatingV1BuildCodeProvider
{
    public ToolChain Provide(string v1PlatformName)
    {
        if (v1PlatformName != "webgl")
            return null;

        return new EmscriptenToolchain(new UnityEmscriptenSdk(EmscriptenArchitecture.AsmJs));
    }
}
