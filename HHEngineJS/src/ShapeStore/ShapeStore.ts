import {TimelineEventFactory, TimelineEventNames, TimelineTrack} from "hhtimeline"
import {v4 as uuidv4} from 'uuid';
import {Logger} from "hhcommoncomponents"
import {BaseShape} from "../Shapes/BaseShape";

class FrameObject{

}

class Layer{
    private shapes: Array<BaseShape>
    public addShape(shape: BaseShape){
        this.shapes.push(shape)
    }
}

// We might have compounded elements. So we might have more than one store in the scene.
// Every shape store might contain multiple tracks/layers.
class ShapeStore{
    trackLayerMap: Map<uuidv4, Layer> = new Map()
    currentLayer: Layer

    constructor() {
        let emitter = TimelineEventFactory.getInstance().getEventEmitter()
        emitter.on(TimelineEventNames.NEWTRACKADDED, this.onNewTrackAdded.bind(this))
    }

    onNewTrackAdded(track: TimelineTrack){
        let newLayer = new Layer()
        this.trackLayerMap.set(track.getId(), newLayer)
        this.currentLayer = newLayer
    }

    getCurrentLayer(){
        return this.currentLayer
    }
}

class ShapeStoreManager{
    static _instance: ShapeStoreManager
    static getInstance(){
        if(!this._instance)
            this._instance = new ShapeStoreManager()
        return this._instance
    }

    shapeStoreMap:Map<uuidv4, ShapeStore>
    currentStore: ShapeStore

    createStore(id:uuidv4){
        let newStore = new ShapeStore()
        this.shapeStoreMap.set(id, newStore)
        this.currentStore = newStore
    }

    getStore(id:uuidv4 = null){
        if(!id)
            return this.currentStore
        if(!this.shapeStoreMap.has(id)){
            Logger.error("Can't find the id:" + id)
        }else{
            return this.shapeStoreMap.get(id)
        }
    }
}

export {ShapeStore, ShapeStoreManager, Layer}