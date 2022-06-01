import { TypedEmitter } from 'tiny-typed-emitter';
import {TimelineTrack} from "./TimelineTrack";
import {v4 as uuidv4} from 'uuid';
import {Logger} from "hhcommoncomponents"

enum TimelineEventNames{
    NEWTRACKADDED = "newTrackAdded"
}

export {TimelineEventNames}