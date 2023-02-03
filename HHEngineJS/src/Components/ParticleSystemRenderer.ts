// @ts-nocheck

import {AbstractComponent, Component} from "./AbstractComponent";
import {ParticleSystemJS} from "../Shapes/ParticleSystemJS";

declare var ti: any;

@Component({compatibleShapes: ["ParticleSystemJS"], maxCount: 1})
class ParticleSystemRenderer extends AbstractComponent {

    rendered: boolean = false

    outputImage
    renderKernel

    constructor(rawObj?) {
        super(rawObj);
    }

    initImage(width, height) {
        this.outputImage = ti.Vector.field(4, ti.f32, [width, height])

        ti.addToKernelScope({
            outputImage: this.outputImage,
            outputImageWidth: Math.ceil(width),
            outputImageHeight: Math.ceil(height)
        })

        if (this.renderKernel == null) {
            this.renderKernel = ti.kernel(() => {

                for (let I of ndrange(i32(outputImageWidth), i32(outputImageHeight))) {
                    // outputImage[I] = [random(), random(), random(), 1.0]
                    outputImage[I] = [25.0 / 255.0, 39.0 / 255.0, 77.0 / 255.0, 1.0]
                }
            })
        }
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (!this.rendered) {
            // Only particle system can have this renderer.
            let particleSystem = this.baseShape as ParticleSystemJS
            let p1 = particleSystem.getLeftUp()
            let p2 = particleSystem.getRightDown()

            let width = p2.x - p1.x
            let height = p2.y - p1.y

            this.initImage(width, height)

            this.renderKernel()
            particleSystem.getCanvas().setImage(this.outputImage)

            this.rendered = true
        }
    }
}

export {ParticleSystemRenderer}