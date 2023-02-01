import {AbstractComponent, Component} from "./AbstractComponent";
import {ParticleSystemJS} from "../Shapes/ParticleSystemJS";

@Component({compatibleShapes: ["ParticleSystemJS"], maxCount: 1})
class ParticleSystemRenderer extends AbstractComponent {

    rendered: boolean = false


    constructor(rawObj?) {
        super(rawObj);
    }


    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if(!this.rendered){
            // Only particle system can have this renderer.
            let particleSystem = this.baseShape as ParticleSystemJS
            let p1 = particleSystem.getLeftUp()
            let p2 = particleSystem.getRightDown()

            let width = p2.x - p1.x
            let height = p2.y - p1.y

            // For all of its pixels...
            for (var i = 0; i < width; i++) {
                for (var j = 0; j < height; j++) {
                    // ...set a random color.
                    particleSystem.setPixel(i, j, paper.Color.random());
                }
            }

            this.rendered = true
        }

    }
}

export {ParticleSystemRenderer}