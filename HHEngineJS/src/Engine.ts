import * as paper from "paper"
import {view} from "paper";
import {Logger, Vector2} from "hhcommoncomponents"
import {ShapeStoreManager} from "./ShapeStore/ShapeStore";
import {v4 as uuidv4} from "uuid"

class EngineJS{
    private static _instance: EngineJS = null

    getInstance(){
        if(!EngineJS._instance)
            EngineJS._instance = new EngineJS()

        return EngineJS._instance
    }

    private canvas: HTMLCanvasElement = null
    private bgRect: paper.Path.Rectangle;
    private contentRect: paper.Path.Rectangle;
    private bgLayer: paper.Layer
    private contentLayer: paper.Layer

    private isPlayer = false
    private aspectRatio:number = 4/3  //  W:H = 4:3

    setIsPlayer(){
        this.isPlayer = true
    }
    getCanvasWidth(){
        return this.canvas.width
    }

    getCanvasHeight(){
        return this.canvas.height
    }

    getProject(){
        return paper.project
    }

    activateBgLayer(){
        if(!this.bgLayer){
            this.bgLayer = new paper.Layer();
            this.contentLayer = this.getProject().activeLayer
            this.getProject().insertLayer(0, this.bgLayer)
        }
        this.bgLayer.activate()
    }

    restoreContentLayer(){
        this.contentLayer.activate()
    }

    createViewRectangle(x, y, w, h, fillColor){
        this.activateBgLayer()
        // Convert all points from view->project coordinate
        let leftUp = view.viewToProject(new paper.Point(x,y))
        let rightDown = view.viewToProject(new paper.Point(x + w, y + h))

        return new paper.Path.Rectangle({
            point:[leftUp.x, leftUp.y],
            size:[rightDown.x - leftUp.x, rightDown.y - leftUp.y],
            strokeColor:'white',
            fillColor: fillColor,
            selected: false
        })
    }

    getContentWH(canvasWidth, canvasHeight){
        let returnWidth = canvasWidth
        let returnHeight = canvasWidth / this.aspectRatio
        if(returnHeight > canvasHeight){
            returnHeight = canvasHeight
            returnWidth = canvasHeight * this.aspectRatio
        }

        return [returnWidth, returnHeight]
    }

    zoomIn(step = 1.05){
        view.zoom *= step;
        this.clearBackground()

        Logger.debug("Current view zoom:" + view.zoom)
    }

    zoomOut(step = 1.05){
        view.zoom /= step;
        this.clearBackground()

        Logger.debug("Current view zoom:" + view.zoom)
    }

    zoomReset(){
        view.zoom = 1.0
        this.clearBackground()
    }

    clearBackground(){

        let canvasWidth = view.element.width
        let canvasHeight = view.element.height
        Logger.debug("Canvas width:" + canvasWidth + ",height:" + canvasHeight)

        if(this.contentRect){
            this.contentRect.remove()
        }

        let [contentWidth, contentHeight] = this.getContentWH(canvasWidth, canvasHeight)
        let xOffset = (canvasWidth - contentWidth)/2
        let yOffset = (canvasHeight - contentHeight)/2
        let leftUp = view.viewToProject(new paper.Point(xOffset, yOffset))
        this.contentRect = this.createViewRectangle(xOffset, yOffset, contentWidth, contentHeight, "white")
        this.contentRect.sendToBack()

        if(!this.isPlayer)
        {
            if(this.bgRect){
                this.bgRect.remove()
            }

            this.bgRect = this.createViewRectangle(0,0, canvasWidth, canvasHeight, paper.Color.random())
            this.bgRect.sendToBack()
        }

        this.restoreContentLayer()
    }

    getWorldPosFromView(x:number, y:number):Vector2{
        let worldPos = paper.view.viewToProject(new paper.Point(x,y))
        return new Vector2(worldPos.x, worldPos.y)
    }

    init(canvas: HTMLCanvasElement){
        this.canvas = canvas
        paper.setup(canvas)

        this.clearBackground()

        // Init a default store
        ShapeStoreManager.getInstance().createStore(uuidv4())
    }
}

export {EngineJS}