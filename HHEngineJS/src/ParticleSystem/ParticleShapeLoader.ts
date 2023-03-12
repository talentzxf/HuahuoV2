// @ts-nocheck
import {huahuoEngine} from "../EngineAPI";

const MAX_PARTICLE_SHAPE_SIZE = 1024

const PARTICLE_TEXTURE_SIZE = 256

class ParticleShapeLoader {
    hiddenImage: HTMLImageElement
    hiddenCanvas: HTMLCanvasElement
    lastParticleShapeResourceName: string = ""

    _particleShapeData
    _particleShapeSize
    _particleShapeTexture

    constructor() {
        this._particleShapeSize = huahuoEngine.ti.field(huahuoEngine.ti.i32, [2])
        this._particleShapeSize.fromArray1D([0, 0])

        this._particleShapeData = huahuoEngine.ti.Vector.field(4, huahuoEngine.ti.f32, [MAX_PARTICLE_SHAPE_SIZE, MAX_PARTICLE_SHAPE_SIZE])

        this._particleShapeTexture = huahuoEngine.ti.texture(4, [256, 256])

        let testTexture = huahuoEngine.ti.classKernel(this, (particleTextureSize)=>{
            for(let I of ndrange(particleTextureSize, particleTextureSize)){
                ti.textureStore(this._particleShapeTexture, I, [0.0, 0.0, 0.0, 0.0])
            }
        })

        testTexture(PARTICLE_TEXTURE_SIZE)
    }

    setParticleShape(binaryResource) {
        if (binaryResource.GetResourceName() != this.lastParticleShapeResourceName) {
            if (this.hiddenImage == null) {
                this.hiddenImage = document.createElement("img")
                this.hiddenImage.id = "particleShapeImg"
            }

            let dataLength = binaryResource.GetDataSize()
            if (dataLength > 0) {
                let ab = new ArrayBuffer(dataLength)
                let binaryData: Uint8Array = new Uint8Array(ab)
                for (let idx = 0; idx < dataLength; idx++) {
                    binaryData[idx] = binaryResource.GetDataAtIndex(idx)
                }

                let mimeType: string = binaryResource.GetMimeType()
                let blob = new Blob([binaryData], {'type': mimeType})
                let reader = new FileReader()
                reader.readAsDataURL(blob)

                let _this = this
                reader.onload = function () {
                    _this.hiddenImage.src = reader.result as string
                    _this.hiddenImage.onload = _this.onImageLoaded.bind(_this)
                }

                this.lastParticleShapeResourceName = binaryResource.GetResourceName()
            } else {
                this._particleShapeSize.fromArray1D([0, 0])
            }
        }
    }

    onImageLoaded() {
        this.hiddenCanvas = document.createElement("canvas")
        this.hiddenCanvas.style.width = this.hiddenImage.width + "px"
        this.hiddenCanvas.style.height = this.hiddenImage.height + "px"
        this.hiddenCanvas.width = this.hiddenImage.width
        this.hiddenCanvas.height = this.hiddenImage.height

        let context = this.hiddenCanvas.getContext("2d")
        context.drawImage(this.hiddenImage, 0, 0)

        let particleShapeImageData = context.getImageData(0, 0, MAX_PARTICLE_SHAPE_SIZE, MAX_PARTICLE_SHAPE_SIZE)

        this._particleShapeSize.fromArray1D([this.hiddenImage.width, this.hiddenImage.height])
        this._particleShapeData.fromArray1D(particleShapeImageData.data)

        let loadParticleTextureKernel = huahuoEngine.ti.classKernel(this, (particleTextureSize)=>{
            let xScale = this._particleShapeSize[0] / particleTextureSize
            let yScale = this._particleShapeSize[1] / particleTextureSize
            for(let I of ndrange(particleTextureSize, particleTextureSize)){
                let mappedI = i32([ I[1] * xScale, I[0] * yScale])
                ti.textureStore(this._particleShapeTexture, I, this._particleShapeData[mappedI])
            }
        })

        loadParticleTextureKernel(PARTICLE_TEXTURE_SIZE)
    }
}

export {ParticleShapeLoader}