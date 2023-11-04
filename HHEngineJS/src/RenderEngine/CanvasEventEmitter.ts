import {EventEmitter, EventParam, GraphEvent, PropertyType} from "hhcommoncomponents";
import * as paper from "paper"

class CanvasEventEmitter extends EventEmitter {
    getEventEmitterName() {
        return "Canvas"
    }

    @GraphEvent(true)
    onMouseMove(@EventParam(PropertyType.VECTOR2) point: paper.Point) {

    }

    @GraphEvent(true)
    onMouseDown(@EventParam(PropertyType.VECTOR2) point: paper.Point) {

    }

    @GraphEvent(true)
    onMouseUp(@EventParam(PropertyType.VECTOR2) point: paper.Point) {

    }
}

export {CanvasEventEmitter}