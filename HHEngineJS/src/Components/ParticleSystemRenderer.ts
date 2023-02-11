// @ts-nocheck

import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {ParticleSystemJS} from "../Shapes/ParticleSystemJS";
import {PropertyCategory} from "./PropertySheetBuilder";
import {huahuoEngine} from "../EngineAPI";

@Component({compatibleShapes: ["ParticleSystemJS"], maxCount: 1})
class ParticleSystemRenderer extends AbstractComponent {
    inited: boolean = false

    outputImage
    renderKernel

    htmlCanvas
    taichiCanvas

    _backgroundColor

    @PropertyValue(PropertyCategory.interpolateColor, {random: true})
    backgroundColor

    _particleNumbers

    @PropertyValue(PropertyCategory.interpolateFloat, 10)
    particleNumbers


    maxNumbers = 10000 // Preload 10000 particles.

    _particleStatuses // 0 - inactive, 1 - active
    _particlePositions
    _particleVelocity

    async initImage(width, height) {
        let widthInt = Math.ceil(width)
        let heightInt = Math.ceil(height)

        this.outputImage = huahuoEngine.ti.Vector.field(4, ti.f32, [widthInt, heightInt])
        this._backgroundColor = huahuoEngine.ti.Vector.field(4, ti.f32, [1])
        this._particleNumbers = huahuoEngine.ti.field(ti.f32, [1])

        this._particleVelocity = huahuoEngine.ti.Vector.field(3, ti.f32, [this.maxNumbers])
        this._particlePositions = huahuoEngine.ti.Vector.field(3, ti.f32, [this.maxNumbers])
        this._particleStatuses = huahuoEngine.ti.field(ti.f32, [this.maxNumbers])

        await this._backgroundColor.set([0], [25.0 / 255.0, 39.0 / 255.0, 77.0 / 255.0, 1.0])

        huahuoEngine.ti.addToKernelScope({
            outputImage: this.outputImage,
            outputImageWidth: widthInt,
            outputImageHeight: heightInt,
            backgroundColor: this._backgroundColor,
            particleNumbers: this._particleNumbers
        })

        if (this.renderKernel == null) {
            this.createRenderKernel()
        }

        if(this.taichiCanvas == null){
            this.htmlCanvas = document.createElement("canvas")
            this.htmlCanvas.width = widthInt
            this.htmlCanvas.height = heightInt
            this.htmlCanvas.style.width = widthInt + "px"
            this.htmlCanvas.style.height = heightInt + "px"
            this.taichiCanvas = new huahuoEngine.ti.Canvas(this.htmlCanvas)

            document.body.append(this.htmlCanvas)
            this.htmlCanvas.style.position = "absolute"
            this.htmlCanvas.style.top = "0px"
            this.htmlCanvas.style.left = "0px"
        }
    }

    async afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        // Only particle system can have this renderer.
        let particleSystem = this.baseShape as ParticleSystemJS

        if (!this.inited) {
            let p1 = particleSystem.getLeftUp()
            let p2 = particleSystem.getRightDown()

            let width = p2.x - p1.x
            let height = p2.y - p1.y

            await this.initImage(width, height)

            this.inited = true
        }

        await this._particleNumbers.set([0], this.particleNumbers)
        await this._backgroundColor.set([0], [this.backgroundColor.red, this.backgroundColor.green,
            this.backgroundColor.blue, this.backgroundColor.alpha])

        this.renderKernel()
        this.taichiCanvas.setImage(this.outputImage)
        let particleSystemCanvasCtx = particleSystem.getCanvas().getContext("2d")
        particleSystemCanvasCtx.drawImage(this.htmlCanvas, 0, 0)
    }

    createRenderKernel(){
        this.renderKernel = huahuoEngine.ti.kernel(() => {

            for (let I of ndrange(i32(outputImageWidth), i32(outputImageHeight))) {
                // outputImage[I] = [random(), random(), random(), 1.0]
                outputImage[I] = backgroundColor[0]
            }
        })
    }
}

export {ParticleSystemRenderer}