import {EventEmitter} from "hhcommoncomponents";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import MouseEvent = paper.MouseEvent;
import {GraphEvent, PropertyType, EventParam} from "hhcommoncomponents";

class BaseShapeEvents extends EventEmitter{
    targetShape
    constructor(targetShape:BaseShapeJS) {
        super();
        this.targetShape = targetShape

        let _this = this

        this.targetShape.executeAfterPaperItemReady((paperItem)=>{
            let mouseEvents = ["onMouseMove", "onMouseDown", "onMouseLeave", "onMouseEnter"]

            for(let mouseEvent of mouseEvents){
                paperItem[mouseEvent] = (evt:MouseEvent)=>{
                    _this[mouseEvent](evt.point.x, evt.point.y)
                }
            }
        })
    }

    @GraphEvent()
    onMouseMove(@EventParam(PropertyType.NUMBER) mouseX, @EventParam(PropertyType.NUMBER) mouseY){
    }

    @GraphEvent()
    onMouseDown(@EventParam(PropertyType.NUMBER) mouseX, @EventParam(PropertyType.NUMBER) mouseY){
    }

    @GraphEvent()
    onMouseLeave(@EventParam(PropertyType.NUMBER) mouseX, @EventParam(PropertyType.NUMBER) mouseY){
    }

    @GraphEvent()
    onMouseEnter(@EventParam(PropertyType.NUMBER) mouseX, @EventParam(PropertyType.NUMBER) mouseY){
    }
}

export {BaseShapeEvents}