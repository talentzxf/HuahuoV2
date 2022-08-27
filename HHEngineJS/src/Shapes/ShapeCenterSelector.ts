import {BaseShapeJS} from "./BaseShapeJS";
import {PropertySheet} from "hhcommoncomponents";

class ShapeCenterSelector{
    private _targetObj: BaseShapeJS;
    private propertySheet: PropertySheet
    private _isSelected: boolean = false;

    private circleShape: paper.Path
    constructor(targetObj: BaseShapeJS) {
        this._targetObj = targetObj

        this.propertySheet = new PropertySheet();

        this._targetObj.registerValueChangeHandler("position")(this.setPosition.bind(this))
    }

    get position(){
        return this.paperShape.position
    }

    set position(val:paper.Point){
        this.paperShape.position = val

        this._targetObj.centerPosition = val
    }

    setPosition(val:paper.Point){
        if(this._isSelected){
            this.position = this._targetObj.centerPosition
        }
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

    set selected(val: boolean){
        if(!val){
            if(this.circleShape){
                this.circleShape.remove()
                this.circleShape = null
            }
        }else{
            this.paperShape
        }

        this._isSelected = val
    }

    update(){
    }

    getPropertySheet(){
        return this.propertySheet
    }

    store(){

    }

    getBornStoreId(){
        return this._targetObj.getBornStoreId()
    }
}

export {ShapeCenterSelector}