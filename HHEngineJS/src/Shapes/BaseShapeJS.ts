import {huahuoEngine} from "../EngineAPI";
import paper from "paper";

declare var Module: any;

class BaseShapeJS
{
    protected rawObj: any = null;

    getShapeName(){
        return "UnknownShape";
    }

    constructor(rawObj?) {
        if(!rawObj)
        {
            let _this = this
            huahuoEngine.ExecuteAfterInited(()=>{
                _this.rawObj = Module.BaseShape.prototype.CreateShape(_this.getShapeName());

                _this.afterWASMReady();
            })
        }else{
            this.rawObj = rawObj
            this.afterWASMReady()
        }
    }

    afterWASMReady(){

    }

    getRawShape(){
        return this.rawObj
    }

    update(){

    }

    getPaperPoint(engineV3Point){
        return new paper.Point(engineV3Point.x, engineV3Point.y)
    }
}

class ShapeFactory{
    shapeNameClzMap: Map<string, Function> = new Map<string, Function>();

    RegisterClass(shapeName:string, shapeConstructor:Function){
        this.shapeNameClzMap.set(shapeName, shapeConstructor)
    }

    GetShapeConstructor(shapeName:string){
        return this.shapeNameClzMap.get(shapeName)
    }
}

let shapeFactory = new ShapeFactory()

huahuoEngine.ExecuteAfterInited(()=>{
    let baseShapeOnLoadHandler = new Module.ScriptEventHandlerImpl()
    baseShapeOnLoadHandler.handleEvent = function(baseShapeEventHandler){
        let arg = Module.wrapPointer(baseShapeEventHandler, Module.ShapeLoadedEventArgs)
        let baseShape = arg.GetBaseShape();

        // Convention: Cpp class name is the JS class name.
        // TODO: Create a map of the shapename->JS class name mapping.

        let shapeConstructor = shapeFactory.GetShapeConstructor(baseShape.GetName())
        let newBaseShape = shapeConstructor(arg.GetBaseShape())

        newBaseShape.update()
    }

    huahuoEngine.GetInstance().RegisterEvent("OnShapeLoaded", baseShapeOnLoadHandler)
})

export {BaseShapeJS, shapeFactory}