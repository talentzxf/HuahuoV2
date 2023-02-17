import {huahuoEngine} from "../EngineAPI";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {BaseShapeJS} from "./BaseShapeJS";

declare var Module: any;

function LoadComponentForShape(shape:BaseShapeJS){
    let baseShape = shape.getRawShape()

    // Create all the component wrapper in the JS side.
    let componentCount = baseShape.GetFrameStateCount()
    for(let idx = 0; idx < componentCount; idx++){
        let componentRawObj = baseShape.GetFrameState(idx)
        let componentConstructor = clzObjectFactory.GetClassConstructor(componentRawObj.GetTypeName())
        if(componentConstructor){
            let component = new componentConstructor(componentRawObj)
            // The component has already been persistented, no need to persistent again.
            shape.addComponent(component, false)

            let subComponentCount = componentRawObj.GetSubComponentCount()
            for(let subComponentIdx = 0 ; subComponentIdx < subComponentCount; subComponentCount++){
                let subComponentRawObj = componentRawObj.GetSubComponentByIdx(subComponentIdx)
                let subComponentConstructor = clzObjectFactory.GetClassConstructor(subComponentRawObj.GetTypeName())
                let subComponent = new subComponentConstructor(subComponentRawObj)
                component.addSubComponent(subComponent)
            }
        }
    }
}

function LoadShapeFromCppShape(rawShapeObj, awake: boolean = true){
    let shapeConstructor = clzObjectFactory.GetClassConstructor(rawShapeObj.GetTypeName())
    let jsShape = shapeConstructor(rawShapeObj)

    // Create all the component wrapper in the JS side.
    LoadComponentForShape(jsShape)

    jsShape.awakeFromLoad()
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
        if(shapeStoreId < 0)
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
        let jsShape = LoadShapeFromCppShape(baseShape)
        let layer = jsShape.getLayer()
        huahuoEngine.getActivePlayer().getLayerShapes(layer).set(jsShape.getRawShape().ptr, jsShape)
    }

    huahuoEngine.GetInstance().RegisterEvent(eventName, baseShapeOnLoadHandler)
})

export {LoadShapeFromCppShape}
