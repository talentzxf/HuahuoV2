import {huahuoEngine} from "../EngineAPI";
import {Logger} from "hhcommoncomponents"
import {Vector2} from "hhcommoncomponents"
import * as paper from "paper";

declare var Module: any;

function relaxRectangle(rectangle, margin) {
    let retRectangle = rectangle.clone()
    retRectangle.x -= margin
    retRectangle.y -= margin
    retRectangle.width += 2 * margin
    retRectangle.height += 2 * margin

    return retRectangle
}

const BOUNDMARGIN:number = 10

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

    store(storeOptions){
        if(storeOptions.segments)
        {
            let segments = this.paperShape.segments
            if(segments){
                let segmentBuffer = []

                for(let id=0; id < segments.length; id++){
                    segmentBuffer[6*id] = segments[id].point.x
                    segmentBuffer[6*id + 1] = segments[id].point.y
                    segmentBuffer[6*id + 2 ] = segments[id].handleIn.x
                    segmentBuffer[6*id + 3 ] = segments[id].handleIn.y
                    segmentBuffer[6*id + 4] = segments[id].handleOut.x
                    segmentBuffer[6*id + 5] = segments[id].handleOut.y
                }
                this.rawObj.SetSegments(segmentBuffer, segments.length)
            }
        }

        if(storeOptions.position){
            this.position = this.paperShape.position

            console.log("Storing position:" + this.position.x + "," + this.position.y)
        }

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

    beforeUpdate(updateOptions){
        if(this.isPermanent && !this.rawObj.IsVisible()){
            this.selected = false
        }
    }

    createShape(){
        throw "Can't create abstract shape, override this function."
    }

    duringUpdate(updateOptions)
    {
        if(updateOptions && updateOptions.updateShape){
            if(!this.paperShape){
                this.createShape()
            }
        }
    }

    applySegments(){
        let segmentCount = this.rawObj.GetSegmentCount();
        if(segmentCount > 0 && this.paperShape.segments.length > 0){
            let currentSegmentCount = this.paperShape.segments.length
            let createSegments = false
            if(currentSegmentCount != segmentCount){
                this.paperShape.removeSegments()
                createSegments = true
            }

            for(let i = 0; i < segmentCount; i++){
                let position = this.rawObj.GetSegmentPositions(i);
                let handleIn = this.rawObj.GetSegmentHandleIns(i);
                let handleOut = this.rawObj.GetSegmentHandleOuts(i);

                let positionPoint = new paper.Point(position.x, position.y)
                let handleInPoint = new paper.Point(handleIn.x, handleIn.y)
                let handleOutPoint = new paper.Point(handleOut.x, handleOut.y)

                if(createSegments){
                    this.paperShape.insert(i, new paper.Segment(positionPoint, handleInPoint, handleOutPoint))
                } else {
                    this.paperShape.segments[i].point = positionPoint
                    this.paperShape.segments[i].handleIn = handleInPoint
                    this.paperShape.segments[i].handleOut = handleOutPoint
                }
            }
            return true
        }

        return false
    }

    afterUpdate(updateOptions)
    {
        if(updateOptions && updateOptions.updateShape){
            let scale = this.rawObj.GetScale()
            this.paperShape.scaling = new paper.Point(scale.x, scale.y)

            this.applySegments()
            let pos = this.rawObj.GetPosition();
            if(this.paperShape.position.x != pos.x || this.paperShape.position.y != pos.y)
                this.paperShape.position = new paper.Point(pos.x, pos.y);
        }

        if(this.isSelected){
            if(updateOptions && updateOptions.updateBoundingBox){
                if(this.boundingBoxRect)
                    this.boundingBoxRect.remove()

                let boundingBox = this.paperShape.bounds;

                let paperjs = this.getPaperJs()
                this.boundingBoxRect = new paperjs.Path.Rectangle(relaxRectangle(boundingBox, BOUNDMARGIN))
                this.boundingBoxRect.dashArray = [4, 10]
                this.boundingBoxRect.strokeColor = new paper.Color("black")
            }

            this.paperShape.selected = true
        }else{
            this.paperShape.selected = false
            if(updateOptions && updateOptions.updateBoundingBox){
                if(this.boundingBoxRect)
                    this.boundingBoxRect.remove()
            }
        }
    }

    update(updateOptions = {updateShape: true, updateBoundingBox : true}){
        this.beforeUpdate(updateOptions)
        this.duringUpdate(updateOptions)

        if(this.isPermanent == true && !this.rawObj.IsVisible()){
            this.paperShape.visible = false
            this.isSelected = false
            this.paperShape.selected = false
        }else{
            this.paperShape.visible = true
            this.afterUpdate(updateOptions)
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