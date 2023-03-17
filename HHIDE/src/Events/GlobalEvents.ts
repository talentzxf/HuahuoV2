// import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
// import {TypedEmitter} from "tiny-typed-emitter";
// import {PropertySheet} from "hhcommoncomponents"
// import {HHContent} from "hhpanel";
//

import {eventBus} from "hhcommoncomponents"
import {huahuoEngine} from "hhenginejs";

enum EventNames{
    DRAWSHAPEBEGINS = 'drawShapeBegins',
    DRAWSHAPEENDS = "drawShapeEnds",
    OBJECTSELECTED = "objectSelected",
    UNSELECTOBJECTS = "unselectObjects",
    COMPONENTADDED = "componentAdded",
    CELLCLICKED = "cellClicked",
    OBJECTDELETED = "objectDeleted"
}

class IDEEventBus {
    private static _instance:IDEEventBus = null
    private ideEventNameSpace: string= "IDE"
    public static getInstance(){
        if(IDEEventBus._instance == null)
            IDEEventBus._instance = new IDEEventBus()
        return IDEEventBus._instance
    }

    on(evtName: string, func){
        huahuoEngine.registerEventListener(this.ideEventNameSpace, evtName, func)
    }

    emit(evtName: string, ...param){
        eventBus.dispatchEvent(this.ideEventNameSpace, evtName, ...param)
    }
}

export {IDEEventBus, EventNames}