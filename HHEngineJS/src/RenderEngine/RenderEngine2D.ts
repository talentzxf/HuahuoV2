import {Vector2} from "hhcommoncomponents"

interface RenderEngine2D{
    init(canvas):void;
    clearBackground():void;
    getWorldPosFromView(x:number, y:number):Vector2
    zoomIn(scale:number):void
    zoomOut(scale:number):void
    zoomReset():void
    resize(width:number, height:number): void
}

export {RenderEngine2D}