import {EventEmitter, EventParam, GraphEvent, PropertyType} from "hhcommoncomponents";
import * as paper from "paper"

class CanvasEventEmitter extends EventEmitter {
    static getEventEmitterName() {
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

    @GraphEvent(true)
    onKeyUp(@EventParam(PropertyType.STRING) key: string, @EventParam(PropertyType.STRING) code){

    }

    @GraphEvent(true)
    onKeyDown(@EventParam(PropertyType.STRING) key: string, @EventParam(PropertyType.STRING) code){

    }
}

export {CanvasEventEmitter}