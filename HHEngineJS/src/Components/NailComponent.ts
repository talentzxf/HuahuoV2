import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {getNailManager} from "../IK/NailManager";
import {Nail} from "../IK/Nail";

const eps:number = 0.001;

@Component({compatibleShapes:["BaseSolidShape"], maxCount: 1})
class NailComponent extends AbstractComponent{
    nails: Array<Nail> = new Array<Nail>()

    lastRotation: number = null

    // The coordinate of this hitPoint is in global world pos.
    addNail(nail: Nail){
        this.nails.push(nail)
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let currentFrame = this.baseShape.getLayer().GetCurrentFrame()
        getNailManager().updateAllNails(currentFrame)

        // After update, the shape has returned to it's original rotation. Need to rotate back.
        if(this.lastRotation){
            this.baseShape.paperShape.rotation = this.lastRotation
        }

        // Adjust position and rotation to reflect the nail change.
        for(let nail of this.nails){
            let nailLocalPosition = nail.getNailLocalLocation(this.baseShape)
            let prevNailGlobalPosition = this.baseShape.localToGlobal(nailLocalPosition)

            let currentNailGlobalPosition = nail.position

            if(prevNailGlobalPosition.getDistance(currentNailGlobalPosition) <= eps){
                continue; // The position is not changed, no need to update my position
            }

            let vector = currentNailGlobalPosition.subtract(prevNailGlobalPosition)
            this.baseShape.paperShape.rotation = vector.angle

            console.log("Angle:" + vector.angle)
            console.log("Directed Angle:" + prevNailGlobalPosition.getDirectedAngle(currentNailGlobalPosition))
            this.baseShape.paperShape.position = this.baseShape.paperShape.position.add(vector)

            this.lastRotation = this.baseShape.paperShape.rotation
        }
    }
}

export {NailComponent}