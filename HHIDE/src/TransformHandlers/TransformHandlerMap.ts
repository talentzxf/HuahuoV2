import {shapeHandlerMoveHandler, shapeMorphHandler, shapeInsertSegmentHandler} from "./ShapeMorphHandler";
import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {shapeTranslateHandler} from "./ShapeTranslateHandler";

class TransformHandlerMap{
    static defaultTransformHandler: ShapeTranslateMorphBase = shapeTranslateHandler
    transformHandlerMap =  {
        "fill" : TransformHandlerMap.defaultTransformHandler,
        "segment": shapeMorphHandler,
        "default": TransformHandlerMap.defaultTransformHandler,
        "handle-in": shapeHandlerMoveHandler,
        "handle-out": shapeHandlerMoveHandler,
        "stroke" : shapeInsertSegmentHandler,
    }
    constructor() {
    }

    getHandler(hitType){
        let defaultHandler = this.transformHandlerMap["default"]

        let retHandler = this.transformHandlerMap[hitType]
        if(!retHandler){
            retHandler = defaultHandler
        }

        return retHandler
    }
}

export {TransformHandlerMap}