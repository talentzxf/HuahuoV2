import {TextureBase} from "taichi.js/src/data/Texture"
import { Program } from "taichi.js/src/program/Program"
import {TextureDimensionality} from "taichi.js/dist/data/Texture";
import {error} from "taichi.js/src/utils/Logging";

export class TransparentCanvasTexture extends TextureBase {
    constructor(public htmlCanvas: HTMLCanvasElement, sampleCount: number) {
        super()

        // Copied from
        Program.getCurrentProgram().runtime!.createGPUCanvasContext(htmlCanvas)
        let context = htmlCanvas.getContext('webgpu')
        if (context === null) {
            error("canvas webgpu context is null")
        }
        let presentationFormat = navigator.gpu.getPreferredCanvasFormat()

        context!.configure({
            device: Program.getCurrentProgram().runtime!.device!,
            format: presentationFormat,
            alphaMode: 'premultiplied'
        })

        this.context = context
        this.format = presentationFormat
        Program.getCurrentProgram().addTexture(this)
        this.sampler = Program.getCurrentProgram().runtime!.createGPUSampler(false, {})
        this.sampleCount = sampleCount
        if (this.sampleCount > 1) {
            this.multiSampledRenderTexture = Program.getCurrentProgram().runtime!.createGPUTexture([htmlCanvas.width, htmlCanvas.height], this.getTextureDimensionality(), this.getGPUTextureFormat(), this.canUseAsRengerTarget(), false, sampleCount)
        }
    }
    multiSampledRenderTexture: GPUTexture | null = null
    context: GPUCanvasContext
    format: GPUTextureFormat
    private sampler: GPUSampler

    getGPUTextureFormat(): GPUTextureFormat {
        return this.format
    }

    canUseAsRengerTarget() {
        return true
    }

    getGPUTexture(): GPUTexture {
        return this.context.getCurrentTexture()
    }

    getGPUTextureView(): GPUTextureView {
        return this.context.getCurrentTexture().createView()
    }

    getGPUSampler(): GPUSampler {
        return this.sampler
    }

    getTextureDimensionality(): TextureDimensionality {
        return TextureDimensionality.Dim2d
    }
}