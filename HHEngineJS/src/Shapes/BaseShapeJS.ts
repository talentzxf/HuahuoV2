import {huahuoEngine} from "../EngineAPI";
import {Logger} from "hhcommoncomponents"
import {Vector2} from "hhcommoncomponents"
import * as paper from "paper";

declare var Module: any;

class BaseShapeJS
{
    protected rawObj: any = null;
    protected paperShape: paper.Path
    protected isSelected = false

    protected boundingBoxRect = null;

    public isPermanent: boolean = false;

    set scale(scale:Vector2){
        this.paperShape.scaling = new paper.Point(scale.x, scale.y)
        this.rawObj.SetScale(scale.x, scale.y, 1.0)
    }

    get scale():Vector2{
        let scale = this.rawObj.GetScale()
        return new Vector2(scale.x, scale.y)
    }


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

    get color():paper.Color{
        let rawObjColor = this.rawObj.GetColor()
        return new paper.Color(rawObjColor.r, rawObjColor.g, rawObjColor.b, rawObjColor.a)
    }

    set color(val:paper.Color){
        this.rawObj.SetColor(val.red, val.green, val.blue, val.alpha)
    }

    getLayer(){
        return this.rawObj.GetLayer()
    }

    awakeFromLoad(){
        this.isPermanent = true
        this.update();
    }

    getShapeName(){
        return "UnknownShape";
    }

    getPaperJs(){
        if(paper.project)
            return paper
        return window.paper
    }

    constructor(rawObj?) {
        if(!rawObj)
        {
            let _this = this

            Logger.info("BaseShapeJS: Submitted execute method")
            huahuoEngine.ExecuteAfterInited(()=>{

                Logger.info("BaseShapeJS: Executing raw obj creation method")
                _this.rawObj = Module.BaseShape.prototype.CreateShape(_this.getShapeName());
                Logger.info("BaseShapeJS: Executing afterWASMReady")
                _this.afterWASMReady();
                Logger.info("BaseShapeJS: Executed afterWASMReady")
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

    isVisible(){
        return this.rawObj.IsVisible();
    }

    beforeUpdate(){
        if(this.isPermanent && !this.rawObj.IsVisible()){
            this.selected = false
        }

        if(!this.isSelected && this.boundingBoxRect){
            this.boundingBoxRect.remove()
        }
    }

    createShape(){
        throw "Can't create abstract shape, override this function."
    }

    duringUpdate()
    {
        if(!this.paperShape){
            this.createShape()
        }
    }

    afterUpdate()
    {
        let scale = this.rawObj.GetScale()
        this.paperShape.scaling = new paper.Point(scale.x, scale.y)

        let pos = this.rawObj.GetPosition();
        this.paperShape.position = new paper.Point(pos.x, pos.y);

        if(this.isSelected){
            if(this.boundingBoxRect)
                this.boundingBoxRect.remove()

            let boundingBox = this.paperShape.bounds;

            let paperjs = this.getPaperJs()
            this.boundingBoxRect = new paperjs.Path.Rectangle(boundingBox)
            this.boundingBoxRect.dashArray = [4, 10]
            this.boundingBoxRect.strokeColor = new paper.Color("black")
        }
    }

    update(){
        this.beforeUpdate()
        this.duringUpdate()

        if(this.isPermanent == true && !this.rawObj.IsVisible()){
            this.paperShape.visible = false
        }else{
            this.paperShape.visible = true
            this.afterUpdate()
        }
    }

    getPaperPoint(engineV3Point){
        return new paper.Point(engineV3Point.x, engineV3Point.y)
    }

    getPaperShape():paper.Path{
        return this.paperShape
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

        let layer = newBaseShape.getLayer()
        huahuoEngine.getLayerShapes(layer).push(newBaseShape)
    }

    huahuoEngine.GetInstance().RegisterEvent("OnShapeLoaded", baseShapeOnLoadHandler)
})

export {BaseShapeJS, shapeFactory}