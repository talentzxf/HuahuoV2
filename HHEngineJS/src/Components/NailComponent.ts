import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {getNailManager} from "../IK/NailManager";
import {Nail} from "../IK/Nail";

const eps: number = 0.001;

@Component({compatibleShapes: ["BaseSolidShape"], maxCount: 1})
class NailComponent extends AbstractComponent {
    nails: Array<Nail> = new Array<Nail>()

    // The coordinate of this hitPoint is in global world pos.
    addNail(nail: Nail) {
        this.nails.push(nail)
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let currentFrame = this.baseShape.getLayer().GetCurrentFrame()
        getNailManager().updateAllNails(currentFrame)

        // Adjust position and rotation to reflect the nail change.
        for (let nail of this.nails) {
            let nailLocalPosition = nail.getNailLocalLocation(this.baseShape)
            let prevNailGlobalPosition = this.baseShape.localToGlobal(nailLocalPosition)
            let currentNailGlobalPosition = nail.position

            if (prevNailGlobalPosition.getDistance(currentNailGlobalPosition) <= eps) {
                continue; // The position is not changed, no need to update my position
            }

            // The nail is in the center of the shape, don't know where to rotate in this case. So don't move
            if(currentNailGlobalPosition.getDistance(this.baseShape.position) <= eps){
                continue; // The position is not changed, no need to update my position
            }

            let nailVector = nailLocalPosition.subtract(this.baseShape.globalToLocal(this.baseShape.position))
            let nailTheta = nailVector.angle

            let vector = currentNailGlobalPosition.subtract(this.baseShape.position)
            this.baseShape.setRotation(vector.angle - nailTheta, false, false)

            let afterNailPosition = this.baseShape.localToGlobal(nailLocalPosition)

            let nailOffset = currentNailGlobalPosition.subtract(afterNailPosition)
            this.baseShape.setParentLocalPosition(this.baseShape.position.add(nailOffset), true, false)
        }
    }
}

export {NailComponent}