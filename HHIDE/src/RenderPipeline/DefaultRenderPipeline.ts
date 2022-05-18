
declare var RenderPipelineImpl:any;
declare var CommandBuffer: any;

class DefaultRenderPipeline{
    constructor() {
        let renderPipelineImpl = new RenderPipelineImpl();
        renderPipelineImpl.render = this.render.bind(this)
    }

    render(context){
        var cmd = new CommandBuffer();
        cmd.ClearRenderTarget(true, true, 0);
        context.ExecuteCommandBuffer(cmd);
        cmd.Release();

        context.DrawRenderers();
        context.Submit();
    }
}

export {DefaultRenderPipeline}