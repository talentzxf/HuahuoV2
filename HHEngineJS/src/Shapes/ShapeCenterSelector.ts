import {BaseShapeJS} from "./BaseShapeJS";
import {PropertySheet} from "hhcommoncomponents";

class ShapeCenterSelector{
    private _targetObj: BaseShapeJS;
    private propertySheet: PropertySheet
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
        return this._targetObj.getCenterCircleShape()
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