import {huahuoEngine} from "../EngineAPI";
import {clzObjectFactory} from "../CppClassObjectFactory";

declare var Module: any;

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

        let shapeConstructor = clzObjectFactory.GetClassConstructor(baseShape.GetTypeName())
        let newBaseShape = shapeConstructor(arg.GetBaseShape())

        let layer = newBaseShape.getLayer()
        huahuoEngine.getActivePlayer().getLayerShapes(layer).set(newBaseShape.getRawShape().ptr, newBaseShape)

        // Create all the component wrapper in the JS side.
        let componentCount = baseShape.GetFrameStateCount()
        for(let idx = 0; idx < componentCount; idx++){
            let componentRawObj = baseShape.GetFrameState(idx)
            let componentConstructor = clzObjectFactory.GetClassConstructor(componentRawObj.GetTypeName())
            if(componentConstructor){
                let component = componentConstructor(componentRawObj)
                // The component has already been persistented, no need to persistent again.
                newBaseShape.addComponent(component, false)
            }
        }

        newBaseShape.awakeFromLoad()
    }

    huahuoEngine.GetInstance().RegisterEvent(eventName, baseShapeOnLoadHandler)
})
