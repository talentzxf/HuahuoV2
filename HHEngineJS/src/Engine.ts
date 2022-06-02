import * as paper from "paper"
import {view} from "paper";
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

    createRectangle(x, y, w, h, fillColor){
        return new paper.Path.Rectangle({
            point:[x, y],
            size:[w,h],
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

    clearBackground(){

        let canvasWidth = view.size.width
        let canvasHeight = view.size.height

        if(this.contentRect){
            this.contentRect.remove()
        }

        let [contentWidth, contentHeight] = this.getContentWH(canvasWidth, canvasHeight)
        let xOffset = (canvasWidth - contentWidth)/2
        let yOffset = (canvasHeight - contentHeight)/2
        this.contentRect = this.createRectangle(xOffset, yOffset, contentWidth, contentHeight, "white")
        this.contentRect.sendToBack()

        if(this.isPlayer)
        {
            if(this.bgRect){
                this.bgRect.remove()
            }

            this.bgRect = this.createRectangle(0,0, canvasWidth, canvasHeight, "gray")
            this.bgRect.sendToBack()
        }
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