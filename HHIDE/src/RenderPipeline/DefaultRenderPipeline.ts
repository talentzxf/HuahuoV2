
declare var RenderPipelineImpl:any;
declare var CommandBuffer: any;

class DefaultRenderPipeline{
    constructor() {
        let renderPipelineImpl = new RenderPipelineImpl();
        renderPipelineImpl.render = this.render.bind(this)
    }

    GetCameras(context){
        let cameras = []
        let numCameras = context.GetNumerOfCameras();
        for(let i = 0; i < numCameras; i++){
            cameras.push(context.GetCamera[i])
        }

        return cameras;
    }

    render(context){
        let cameras = this.GetCameras(context)

        var cmd = new CommandBuffer();
        cmd.ClearRenderTarget(true, true, 0);
        context.ExecuteCommandBuffer(cmd);
        cmd.Release();

        for(let camera of cameras){
            let cullingParamaters = camera.TryGetCullingParameters();
            let cullingResults = context.Cull(cullingParamaters)
            context.SetupCameraProperties(camera)
            context.DrawRenderers(cullingResults, null, null);
        }

        context.Submit();
    }
}

export {DefaultRenderPipeline}