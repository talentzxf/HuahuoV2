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

    constructor() {
        for(let evtName of Object.values(EventNames)){
            eventBus.registerEvent(evtName, "IDE")
        }
    }

    on(evtName: string, func){
        eventBus.addEventHandler(evtName, this.ideEventNameSpace, func)
    }

    emit(evtName: string, ...param){
        eventBus.triggerEvent(evtName, this.ideEventNameSpace, ...param)
    }
}

export {EventBus, EventNames}