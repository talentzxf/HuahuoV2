import {TimelineEventFactory, TimelineEventNames, TimelineTrack} from "hhtimeline"
import {v4 as uuidv4} from 'uuid';
import {Logger} from "hhcommoncomponents"
import {BaseShape} from "../Shapes/BaseShape";

class FrameObject{

}

class Layer{
    private shapes: Array<BaseShape> = new Array
    public addShape(shape: BaseShape){
        this.shapes.push(shape)
    }
}

// We might have compounded elements. So we might have more than one store in the scene.
// Every shape store might contain multiple tracks/layers.
class ShapeStore{
    trackLayerMap: Map<uuidv4, Layer>
    currentLayer: Layer

    constructor() {
        this.trackLayerMap = new Map();
    }

    createLayer(trackId: uuidv4){
        let layer = new Layer();
        this.currentLayer = layer;
        this.trackLayerMap.set(trackId, layer)
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

    shapeStoreMap:Map<uuidv4, ShapeStore> = new Map()
    currentStore: ShapeStore

    createStore(id:uuidv4){
        let newStore = new ShapeStore()
        this.shapeStoreMap.set(id, newStore)
        this.currentStore = newStore
    }

    getCurrentStore(){
        if(!this.currentStore){
            this.createStore(uuidv4())
        }

        return this.currentStore
    }

    getStore(id:uuidv4 = null){
        if(!id)
            return this.getCurrentStore()
        if(!this.shapeStoreMap.has(id)){
            Logger.error("Can't find the id:" + id)
        }else{
            return this.shapeStoreMap.get(id)
        }
    }
}

export {ShapeStore, ShapeStoreManager, Layer}