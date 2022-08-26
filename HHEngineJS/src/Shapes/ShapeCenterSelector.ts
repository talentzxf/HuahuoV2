import {BaseShapeJS} from "./BaseShapeJS";

class ShapeCenterSelector{
    private _targetObj: BaseShapeJS;
    constructor(targetObj: BaseShapeJS) {
        this._targetObj = targetObj
    }

    isSelectable() {
        return true
    }
}

export {ShapeCenterSelector}