// @ts-nocheck

import {Component, PropertyValue} from "./AbstractComponent";
import {ParticleSystemJS} from "../Shapes/ParticleSystemJS";
import {PropertyCategory} from "./PropertySheetBuilder";
import {huahuoEngine} from "../EngineAPI";
import {GroupComponent} from "./GroupComponent";
import {SubComponentArrayProperty} from "hhcommoncomponents";
import {Particles} from "../ParticleSystem/Particles";

// Convention: All variables start with _ is taichi variables.
//             All variables with the same name is to receive input from Inspector.
@Component({compatibleShapes: ["ParticleSystemJS"], maxCount: 1})
class ParticleSystemRenderer extends GroupComponent { // Inherit from GroupComponent, because each particle system renderer might contain multiple particles.
    inited: boolean = false

    outputImage
    renderKernel

    htmlCanvas
    taichiCanvas

    renderTarget
    depth

    outputImageWidth
    outputImageHeight

    @PropertyValue(PropertyCategory.subcomponentArray, null, {subComponentTypeName: "Particles"} as SubComponentArrayProperty)
    particleSystems

    addParticles(particles: Particles) {
        let groupComponentRawObj = this.rawObj.GetSubComponentArrayByName("particleSystems")
        let groupComponent = this.getComponentByRawObj(groupComponentRawObj)
        groupComponent.addSubComponent(particles)
    }

    async initImage(width, height) {

        this.outputImageWidth = Math.ceil(width)
        this.outputImageHeight = Math.ceil(height)

        this.outputImage = huahuoEngine.ti.Vector.field(4, ti.f32, [this.outputImageWidth, this.outputImageHeight])

        if (this.renderKernel == null) {
            this.createRenderKernel()
        }

        if (this.taichiCanvas == null) {
            this.htmlCanvas = document.createElement("canvas")

            this.htmlCanvas.width = this.outputImageWidth
            this.htmlCanvas.height = this.outputImageHeight
            this.htmlCanvas.style.width = this.outputImageWidth + "px"
            this.htmlCanvas.style.height = this.outputImageHeight + "px"
            // this.taichiCanvas = new huahuoEngine.ti.Canvas(this.htmlCanvas)

            this.renderTarget = huahuoEngine.ti.canvasTexture(this.htmlCanvas)
            this.depth = huahuoEngine.ti.depthTexture([this.outputImageWidth, this.outputImageHeight])

            huahuoEngine.ti.addToKernelScope({
                depth: this.depth
            })

            // document.body.appendChild(this.htmlCanvas)
            // this.htmlCanvas.style.position = "absolute"
            // this.htmlCanvas.style.top = "0px"
            // this.htmlCanvas.style.left = "0px"
        }
    }

    async afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        // Only particle system can have this renderer.
        let particleSystemRaster = this.baseShape as ParticleSystemJS

        if (!this.inited) {
            let p1 = particleSystemRaster.getLeftUp()
            let p2 = particleSystemRaster.getRightDown()

            let width = p2.x - p1.x
            let height = p2.y - p1.y

            await this.initImage(width, height)

            this.inited = true
        }

        // await parallel
        // await Promise.all([
        //     this._backgroundColor.set([0], [this.backgroundColor.red, this.backgroundColor.green,
        //         this.backgroundColor.blue, this.backgroundColor.alpha])])

        await this.renderKernel()

        let currentFrameId = this.baseShape.getLayer().GetCurrentFrame()

        let aspectRatio = this.outputImageWidth / this.outputImageHeight

        for (let particles of this.particleSystems) {
            if (particles.isComponentActive()){
                particles.depth = this.depth
                particles.renderTarget = this.renderTarget
                particles.renderImage(currentFrameId, aspectRatio)
            }
        }

        // await this.taichiCanvas.setImage(this.outputImage)
        particleSystemRaster.clearAndDrawImage(this.htmlCanvas)
    }

    createRenderKernel() {
        this.renderKernel = huahuoEngine.ti.classKernel(this, () => {

            let center = [this.outputImageWidth / 2.0, this.outputImageHeight / 2.0]

            for (let I of ndrange(i32(this.outputImageWidth), i32(this.outputImageHeight))) {
                // outputImage[I] = [random(), random(), random(), 1.0]

                // let dist = (I - center).norm()
                // if (dist < 100.0) {
                //     outputImage[I] = backgroundColor[0]
                // } else {
                //     outputImage[I] = [0.0, 0.0, 0.0, 0.0]
                // }

                this.outputImage[I] = [0.0, 0.0, 0.0, 0.0]
            }
        })
    }
}

export {ParticleSystemRenderer}