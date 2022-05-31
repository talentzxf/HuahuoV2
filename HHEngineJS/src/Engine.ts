import * as paper from "paper"
import {view} from "paper";

class EngineJS{
    private static _instance: EngineJS = null

    private canvas: HTMLCanvasElement = null

    getInstance(){
        if(!EngineJS._instance)
            EngineJS._instance = new EngineJS()

        return EngineJS._instance
    }

    getCanvasWidth(){
        return this.canvas.width
    }

    getCanvasHeight(){
        return this.canvas.height
    }

    init(canvas: HTMLCanvasElement){
        this.canvas = canvas
        paper.setup(canvas)
        // Create a Paper.js Path to draw a line into it:
        var rect = new paper.Path.Rectangle({
            point:[0,0],
            size:[view.size.width, view.size.height],
            strokeColor: 'white',
            fillColor: 'white',
            selected: false
        })
        rect.sendToBack()
        rect.fillColor = new paper.Color("white")
    }
}

export {EngineJS}