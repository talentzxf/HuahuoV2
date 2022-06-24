import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {TypedEmitter} from "tiny-typed-emitter";
import {PropertySheet} from "hhcommoncomponents"

enum EventNames{
    DRAWSHAPEBEGINS = 'drawShapeBegins',
    DRAWSHAPEENDS = "drawShapeEnds",
    OBJECTSELECTED = "objectSelected"
}

interface GlobalEvents{
    drawShapeBegins:(shapeDrawer: BaseShapeDrawer)=>void;
    drawShapeEnds:(shapeDrawer: BaseShapeDrawer)=>void;
    objectSelected:(shapes: PropertySheet)=>void;
}

class EventBus extends TypedEmitter<GlobalEvents>{
    private static _instance:EventBus = null
    public static getInstance(){
        if(EventBus._instance == null)
            EventBus._instance = new EventBus()
        return EventBus._instance
    }
}

export {EventBus, EventNames}