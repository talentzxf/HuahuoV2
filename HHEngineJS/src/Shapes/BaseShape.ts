import {Vector2} from "hhcommoncomponents"
import {EngineJS} from "../Engine";
import * as paper from "paper"

class BaseShape{
    getGlobalPositionFromVec(p: Vector2):Vector2{
        let vec = this.getGlobalPosition(p.x, p.y)
        return new paper.Point(vec.x, vec.y)
    }

    getGlobalPosition(x: number, y: number):Vector2{
        let width = EngineJS.prototype.getInstance().getCanvasWidth()
        let height = EngineJS.prototype.getInstance().getCanvasHeight()

        return new Vector2( x * width, y * height)
    }

    update(){

    }
}

export {BaseShape}