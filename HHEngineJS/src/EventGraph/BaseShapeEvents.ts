import {EventEmitter} from "hhcommoncomponents";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import MouseEvent = paper.MouseEvent;
import {GraphEvent, PropertyType, EventParam} from "hhcommoncomponents";

class BaseShapeEvents extends EventEmitter{
    targetShape
    constructor(targetShape:BaseShapeJS) {
        super();
        this.targetShape = targetShape

        let paperItem = this.targetShape.paperItem
        paperItem.onMouseMove = this.onMouseMove.bind(this)

        paperItem.onMouseDown = this.onMouseDown.bind(this)

        paperItem.onMouseEnter = this.onMouseEnter.bind(this)
    }

    @GraphEvent()
    mouseMoveEvent(@EventParam(PropertyType.NUMBER) mouseX, @EventParam(PropertyType.NUMBER) mouseY){

    }

    onMouseMove(evt: MouseEvent){
        console.log("MouseMove")
        this.mouseMoveEvent(evt.point.x, evt.point.y)
    }

    onMouseDown(evt: MouseEvent){
        console.log("OnMouseDown")
    }

    onMouseEnter(evt: MouseEvent){
        console.log("OnMouseEnter")
    }
}

export {BaseShapeEvents}