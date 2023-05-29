import {huahuoEngine} from "../EngineAPI";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {Utils} from "./Utils";

declare var Module: any;

function LoadShapeFromCppShape(rawShapeObj, awake: boolean = true, addToLayer: boolean = true, isMirage: boolean = false){
    let shapeConstructor = clzObjectFactory.GetClassConstructor(rawShapeObj.GetTypeName())
    let jsShape = shapeConstructor(rawShapeObj)

    let shapeDecorator = huahuoEngine.getShapeDecorator()
    if(shapeDecorator){
        jsShape = shapeDecorator(jsShape)
    }

    if(addToLayer){
        let layer = jsShape.getLayer()
        huahuoEngine.getActivePlayer().getLayerShapes(layer).set(jsShape.getRawShape().ptr, jsShape)
    }

    jsShape.isMirage = isMirage

    if(awake)
        jsShape.awakeFromLoad()

    // TODO: Whatif there're dependencies across components?
    jsShape.LoadComponents()
    
    return jsShape
}

huahuoEngine.ExecuteAfterInited(()=>{
    let eventName = "OnProjectCompletelyLoaded"
    if(huahuoEngine.GetInstance().IsEventRegistered(eventName)){
        return;
    }

    let projectCompletedLoadedHandler = new Module.ScriptEventHandlerImpl()
    projectCompletedLoadedHandler.handleEvent = function (){
        huahuoEngine.getActivePlayer().updateAllShapes()
    }

    huahuoEngine.GetInstance().RegisterEvent(eventName, projectCompletedLoadedHandler)
})

huahuoEngine.ExecuteAfterInited(() => {
    let eventName = "OnShapeLoaded"
    if (huahuoEngine.GetInstance().IsEventRegistered(eventName))
        return;

    let baseShapeOnLoadHandler = new Module.ScriptEventHandlerImpl()
    baseShapeOnLoadHandler.handleEvent = function (baseShapeEventHandler) {
        let arg = Module.wrapPointer(baseShapeEventHandler, Module.ShapeLoadedEventArgs)
        let baseShape = arg.GetBaseShape();

        let shapeStoreId = baseShape.GetStoreId()
        if(!Utils.isValidGUID(shapeStoreId))
            return;

        if (shapeStoreId != huahuoEngine.GetCurrentStoreId()) {
            let elementShapes = huahuoEngine.GetElementShapeByStoreId(shapeStoreId)
            if (elementShapes) {
                for (let elementShape of elementShapes) {
                    if (elementShape) {
                        elementShape.update();
                    }
                }
            }
            return;
        }

        // Convention: Cpp class name is the JS class name.
        // TODO: Create a map of the shapename->JS class name mapping.
        LoadShapeFromCppShape(baseShape)
    }

    huahuoEngine.GetInstance().RegisterEvent(eventName, baseShapeOnLoadHandler)
})

export {LoadShapeFromCppShape}
