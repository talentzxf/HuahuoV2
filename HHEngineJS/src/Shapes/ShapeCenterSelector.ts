import {BaseShapeJS} from "./BaseShapeJS";
import {PropertySheet} from "hhcommoncomponents";

class ShapeCenterSelector{
    private _targetObj: BaseShapeJS;
    private propertySheet: PropertySheet

    private circleShape: paper.Path
    constructor(targetObj: BaseShapeJS) {
        this._targetObj = targetObj

        this.propertySheet = new PropertySheet();
    }

    get position(){
        return this.paperShape.position
    }

    set position(val:paper.Point){
        this.paperShape.position = val
    }

    get paperShape(){
        if(!this.circleShape){
            let paperJs = this._targetObj.getPaperJs()
            this.circleShape = new paperJs.Path.Circle(this._targetObj.centerPosition, 20)
            this.circleShape.fillColor = new paper.Color("red")
        }
        return this.circleShape
    }

    isSelectable() {
        return true
    }

    getParent(){
        return null
    }

    update(){

    }

    getPropertySheet(){
        return this.propertySheet
    }
}

export {ShapeCenterSelector}