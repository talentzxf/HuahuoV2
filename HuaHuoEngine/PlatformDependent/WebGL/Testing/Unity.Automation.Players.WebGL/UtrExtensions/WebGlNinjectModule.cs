using Ninject.Modules;
using UnityEngine.Common;
using UnityEngine.Common.Enums;

namespace Unity.Automation.Players.WebGL.UtrExtensions
{
    public class WebGLNinjectModule : NinjectModule
    {
        public override void Load()
        {
            Bind<IUnityPlayer>().To<Player>().Named(RuntimePlatform.WebGL.ToString());
            Bind<IUnityPlayerStartInfo>().To<Starter>().Named(RuntimePlatform.WebGL.ToString());
        }
    }
}
