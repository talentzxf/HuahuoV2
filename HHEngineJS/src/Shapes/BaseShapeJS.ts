import {huahuoEngine} from "../EngineAPI";
import {Vector2} from "hhcommoncomponents"
import * as paper from "paper";

declare var Module: any;

class BaseShapeJS
{
    protected rawObj: any = null;
    protected paperShape: paper.Path
    protected isSelected = false

    protected boundingBoxRect = null;

    get selected():boolean{
        return this.isSelected
    }

    set selected(val:boolean){
        this.isSelected = val
    }

    get position():Vector2{
        let point = this.rawObj.GetPosition()
        return new Vector2(point.x, point.y)
    }

    set position(val:Vector2){
        this.rawObj.SetPosition(val.x, val.y, 0);
    }

    awakeFromLoad(){
        this.update();
    }

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

    isSelectable(){
        return true
    }

    afterWASMReady(){

    }

    getRawShape(){
        return this.rawObj
    }

    beforeUpdate(){
        // Clear shapes
        if(this.paperShape){
            this.paperShape.remove()
        }

        if(this.boundingBoxRect){
            this.boundingBoxRect.remove()
        }
    }

    duringUpdate()
    {

    }

    afterUpdate()
    {
        let pos = this.rawObj.GetPosition();

        this.paperShape.position = new paper.Point(pos.x, pos.y);

        if(this.isSelected){
            let boundingBox = this.paperShape.bounds;
            this.boundingBoxRect = new paper.Path.Rectangle(boundingBox)
            this.boundingBoxRect.dashArray = [4, 10]
            this.boundingBoxRect.strokeColor = new paper.Color("black")
        }
    }

    update(){
        this.beforeUpdate()
        this.duringUpdate()
        this.afterUpdate()
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

        newBaseShape.awakeFromLoad()
    }

    huahuoEngine.GetInstance().RegisterEvent("OnShapeLoaded", baseShapeOnLoadHandler)
})

export {BaseShapeJS, shapeFactory}