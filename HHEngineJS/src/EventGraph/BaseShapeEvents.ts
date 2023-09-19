import {EventEmitter} from "hhcommoncomponents";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import MouseEvent = paper.MouseEvent;
import {GraphEvent, PropertyType, EventParam} from "hhcommoncomponents";

class BaseShapeEvents extends EventEmitter {
    targetShape

    constructor(targetShape: BaseShapeJS) {
        super();
        this.targetShape = targetShape

        let _this = this

        this.targetShape.executeAfterPaperItemReady((paperItem) => {
            let mouseEvents = ["onMouseMove", "onMouseDown", "onMouseLeave", "onMouseEnter", "onMouseUp"]

            for (let mouseEvent of mouseEvents) {
                paperItem[mouseEvent] = (evt: MouseEvent) => {
                    _this[mouseEvent](evt.point)
                }
            }

            _this.targetShape.registerValueChangeHandler("onBornFrame")(_this.onBornFrame.bind(_this))
        })
    }

    @GraphEvent()
    onBornFrame() {

    }

    @GraphEvent()
    onMouseMove(@EventParam(PropertyType.VECTOR2) point) {
    }

    @GraphEvent()
    onMouseDown(@EventParam(PropertyType.VECTOR2) point) {
    }

    @GraphEvent()
    onMouseLeave(@EventParam(PropertyType.VECTOR2) point) {
    }

    @GraphEvent()
    onMouseEnter(@EventParam(PropertyType.VECTOR2) point) {
    }

    @GraphEvent()
    onMouseUp(@EventParam(PropertyType.VECTOR2) point) {
    }
}

export {BaseShapeEvents}