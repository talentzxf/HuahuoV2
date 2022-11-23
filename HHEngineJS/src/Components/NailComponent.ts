import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {getNailManager} from "../IK/NailManager";
import {Nail} from "../IK/Nail";

const eps: number = 0.001;

@Component({compatibleShapes: ["BaseSolidShape"], maxCount: 1})
class NailComponent extends AbstractComponent {
    nails: Array<Nail> = new Array<Nail>()

    lastRotation: number = null
    lastPosition: paper.Point = null

    prevNailShape: paper.Path = null
    afterNailShape: paper.Path = null

    // The coordinate of this hitPoint is in global world pos.
    addNail(nail: Nail) {
        this.nails.push(nail)
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        let currentFrame = this.baseShape.getLayer().GetCurrentFrame()
        getNailManager().updateAllNails(currentFrame)

        // // After update, the shape has returned to it's original rotation. Need to rotate back.
        // if(this.lastPosition){
        //     this.baseShape.paperShape.position = this.lastPosition
        // }
        //
        // if (this.lastRotation) {
        //     this.baseShape.paperShape.rotation = this.lastRotation
        // }

        console.log("IK: Shape type:" + this.baseShape.getTypeName())
        // Adjust position and rotation to reflect the nail change.
        for (let nail of this.nails) {
            let nailLocalPosition = nail.getNailLocalLocation(this.baseShape)
            console.log("IK: nailLocalPosition:" + nailLocalPosition.x + "," + nailLocalPosition.y)
            let prevNailGlobalPosition = this.baseShape.localToGlobal(nailLocalPosition)
            console.log("IK:prevNailGlobalPosition:" + prevNailGlobalPosition.x + "," + prevNailGlobalPosition.y)

            let currentNailGlobalPosition = nail.position
            console.log("IK:currentNailGlobalPosition:" + currentNailGlobalPosition.x + "," + currentNailGlobalPosition.y)
            if(!this.prevNailShape){
                this.prevNailShape = new paper.Path.Circle(prevNailGlobalPosition, 10)
                this.prevNailShape.fillColor = new paper.Color("blue")
            }else{
                this.prevNailShape.position = prevNailGlobalPosition
            }

            if (prevNailGlobalPosition.getDistance(currentNailGlobalPosition) <= eps) {
                console.log("IK:SKIPPED")
                this.lastRotation = null
                this.lastPosition = null
                continue; // The position is not changed, no need to update my position
            }

            // The nail is in the center of the shape, don't know where to rotate in this case. So don't move
            if(currentNailGlobalPosition.getDistance(this.baseShape.position) <= eps){
                console.log("IK:SKIPPED, nail in center")
                this.lastRotation = null
                this.lastPosition = null
                continue; // The position is not changed, no need to update my position
            }

            console.log("IK: baseShape.rotation:" + this.baseShape.rotation)
            console.log("IK: baseShape.shapeRotation:" + this.baseShape.paperShape.rotation)
            console.log("IK: baseShape.position:" + this.baseShape.position)
            console.log("IK: baseShape.shapePosition" + this.baseShape.paperShape.position)
            console.log("IK: baseShape.localPosition:" + this.baseShape.globalToLocal(this.baseShape.position))
            console.log("IK: baseShape.localShapePosition:" + this.baseShape.globalToLocal(this.baseShape.shapePosition))

            let nailVector = nailLocalPosition.subtract(this.baseShape.globalToLocal(this.baseShape.position))
            console.log("IK:nailVector:" + nailVector.x + "," + nailVector.y)
            let nailTheta = nailVector.angle
            console.log("IK:nailTheta:" + nailTheta)

            let vector = currentNailGlobalPosition.subtract(this.baseShape.position)
            console.log("IK:vector:" + vector + " angle:" + vector.angle)
            this.baseShape.setRotation(vector.angle - nailTheta, false, false)
            console.log("IK:nailTheta:" + vector.angle +"," + nailTheta + " result:" + (vector.angle - nailTheta))

            let afterNailPosition = this.baseShape.localToGlobal(nailLocalPosition)
            console.log("IK:afterNailPosition:" + afterNailPosition)
            console.log("IK:afterPosition:" + this.baseShape.position)
            console.log("IK:afterShapePosition:" + this.baseShape.shapePosition)

            let nailOffset = currentNailGlobalPosition.subtract(prevNailGlobalPosition)
            this.baseShape.setParentLocalPosition(this.baseShape.position.add(nailOffset), false, false)

            this.baseShape.updatePosition()

            if(!this.afterNailShape){
                this.afterNailShape = new paper.Path.Circle(afterNailPosition, 10)
                this.afterNailShape.fillColor = new paper.Color("yellow")
            }else{
                this.afterNailShape.position = afterNailPosition
            }
        }
    }
}

export {NailComponent}