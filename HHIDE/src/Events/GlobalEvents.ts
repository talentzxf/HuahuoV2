// import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
// import {TypedEmitter} from "tiny-typed-emitter";
// import {PropertySheet} from "hhcommoncomponents"
// import {HHContent} from "hhpanel";
//

import {eventBus} from "hhcommoncomponents"

enum EventNames{
    DRAWSHAPEBEGINS = 'drawShapeBegins',
    DRAWSHAPEENDS = "drawShapeEnds",
    OBJECTSELECTED = "objectSelected",
    UNSELECTOBJECTS = "unselectObjects",
}

class EventBus{
    private static _instance:EventBus = null
    private ideEventNameSpace: string= "IDE"
    public static getInstance(){
        if(EventBus._instance == null)
            EventBus._instance = new EventBus()
        return EventBus._instance
    }

    on(evtName: string, func){
        eventBus.addEventHandler(this.ideEventNameSpace, evtName, func)
    }

    emit(evtName: string, ...param){
        eventBus.triggerEvent(this.ideEventNameSpace, evtName, ...param)
    }
}

export {EventBus, EventNames}