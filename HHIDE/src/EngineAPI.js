import {DefaultRenderPipeline} from "./RenderPipeline/DefaultRenderPipeline";

class EngineAPI{
    static inited = false

    static PendingInitFunctions = []

    static ExecuteAfterInited(func){
        if(EngineAPI.inited){
            return func();
        }

        EngineAPI.PendingInitFunctions.push(func);
    }

    static OnInit(){
        EngineAPI.inited = true

        EngineAPI.PendingInitFunctions.forEach(func=>{
            func();
        })

        // TODO: Load scene from localstorage if it's not empty.
        let sceneManager = EngineAPI.GetInstance().GetSceneManager()
        let scene = sceneManager.CreateScene();
        sceneManager.SetActiveScene(scene);

        let defaultRenderPipeline = new DefaultRenderPipeline()
        EngineAPI.SetRenderer(defaultRenderPipeline)
    }

    static SetRenderer(renderPipeline){
        Module.RenderPipelineManager.prototype.GetManager().SetRenderPipeline(renderPipeline)
    }

    static GetInstance(){
        if(EngineAPI.inited){
            return Module.HuaHuoEngine.prototype.getInstance()
        }else {
            console.log("Please wait while loading engine!")
            throw "Engine not inited!"
        }
    }
}


export {EngineAPI}