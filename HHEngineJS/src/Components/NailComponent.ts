import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {getNailManager} from "../IK/NailManager";
import {Nail} from "../IK/Nail";

const eps: number = 0.001;

@Component({compatibleShapes: ["BaseSolidShape"], maxCount: 1})
class NailComponent extends AbstractComponent {
    nails: Array<Nail> = new Array<Nail>()

    lastRotation: number = null
    lastPosition: paper.Point = null

    // The coordinate of this hitPoint is in global world pos.
    addNail(nail: Nail) {
        this.nails.push(nail)
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let currentFrame = this.baseShape.getLayer().GetCurrentFrame()
        getNailManager().updateAllNails(currentFrame)

        // After update, the shape has returned to it's original rotation. Need to rotate back.
        if (this.lastRotation) {
            this.baseShape.paperShape.rotation = this.lastRotation
        }

        if(this.lastPosition){
            this.baseShape.paperShape.position = this.lastPosition
        }

        // Adjust position and rotation to reflect the nail change.
        for (let nail of this.nails) {
            let nailLocalPosition = nail.getNailLocalLocation(this.baseShape)

            let prevNailGlobalPosition = this.baseShape.localToGlobal(nailLocalPosition)

            let currentNailGlobalPosition = nail.position

            if (prevNailGlobalPosition.getDistance(currentNailGlobalPosition) <= eps) {
                this.lastRotation = this.baseShape.paperShape.rotation
                continue; // The position is not changed, no need to update my position
            }

            let vector = currentNailGlobalPosition.subtract(this.baseShape.position)
            this.baseShape.paperShape.rotation = vector.angle

            let nailOffset = currentNailGlobalPosition.subtract(prevNailGlobalPosition)
            this.baseShape.paperShape.position = this.baseShape.paperShape.position.add(nailOffset)

            this.lastRotation = this.baseShape.paperShape.rotation
            this.lastPosition = this.baseShape.paperShape.position
        }
    }
}

export {NailComponent}