import { TypedEmitter } from 'tiny-typed-emitter';
import {TimelineTrack} from "./TimelineTrack";
import {v4 as uuidv4} from 'uuid';
import {Logger} from "hhcommoncomponents"

enum TimelineEventNames{
    NEWTRACKADDED = "newTrackAdded"
}

interface TimelineEvent {
    'newTrackAdded': (track:TimelineTrack) => void;
}

class TimelineEventFactory{
    static _instance:TimelineEventFactory
    static getInstance(){
        if(!TimelineEventFactory._instance){
            TimelineEventFactory._instance = new TimelineEventFactory()
        }
        return TimelineEventFactory._instance
    }

    private uuidEventMap: Map<uuidv4, TypedEmitter<TimelineEvent>> = new Map
    private currentEventEmitter: TypedEmitter<TimelineEvent>
    createTimelineEvent(timelineId: uuidv4){
        let emitter = new TypedEmitter<TimelineEvent>();
        this.uuidEventMap.set(timelineId, emitter)
        this.currentEventEmitter = emitter
        return emitter
    }

    getEventEmitter(timelineId: uuidv4 = null){
        if(!timelineId){
            return this.currentEventEmitter
        }

        if(!this.uuidEventMap.has(timelineId)){
            Logger.error("Can't find the uuid")
        }else{
            return this.uuidEventMap.get(timelineId)
        }
    }
}

export {TimelineEventFactory, TimelineEvent, TimelineEventNames}
