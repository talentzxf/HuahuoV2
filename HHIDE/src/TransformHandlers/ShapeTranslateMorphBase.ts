import {BaseShapeJS, Vector2} from "hhenginejs"

class ShapeTranslateMorphBase{
    protected curObjs: Array<BaseShapeJS> = null
    protected startPos: Vector2 = null
    protected lastPos: Vector2 = null
    protected isDragging: boolean = false

    getIsDragging(){
        return this.isDragging
    }

    setTarget(objs:Array<BaseShapeJS>){
        this.curObjs = objs
    }

    beginMove(startPos: Vector2){
        this.lastPos = startPos
        this.startPos = startPos
        this.isDragging = true
    }

    dragging(pos){

    }

}
export {ShapeTranslateMorphBase}