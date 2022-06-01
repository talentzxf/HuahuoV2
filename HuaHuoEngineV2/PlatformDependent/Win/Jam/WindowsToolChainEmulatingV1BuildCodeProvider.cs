using System.Diagnostics.CodeAnalysis;
using System.Linq;
using Bee.Core;
using Bee.NativeProgramSupport;
using Bee.Toolchain.Windows;
using Unity.BuildSystem;
using Unity.BuildSystem.VisualStudio;
[SuppressMessage("ReSharper", "UnusedMember.Global")] // used through reflection
class WindowsToolChainEmulatingV1BuildCodeProvider : IToolChainEmulatingV1BuildCodeProvider
{
    public ToolChain Provide(string v1PlatformName)
    {
        switch (v1PlatformName)
        {
            case "win32":
            case "win64":
                var arch = v1PlatformName == "win32" ? (IntelArchitecture) new x86Architecture() : new x64Architecture();
                var toolchain = new WindowsToolchain(WindowsSdk.LocatorFor(arch).UserDefaultOrDummy);
                // MSVC installation might not be present (e.g. gfx test machines)
                if (toolchain.CanBuild)
                    return toolchain;
                return new GenericToolChain(new WindowsPlatform(), arch, v1PlatformName);
            default:
                return null;
        }
    }
}
