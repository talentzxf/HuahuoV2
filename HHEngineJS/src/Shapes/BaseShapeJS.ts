import {huahuoEngine} from "../EngineAPI";

declare var Module: any;

class BaseShapeJS
{
    protected rawObj: any = null;

    getShapeName(){
        return "UnknownShape";
    }

    constructor() {
        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            _this.rawObj = Module.BaseShape.prototype.CreateShape(_this.getShapeName());

            _this.afterWASMReady();
        })
    }

    afterWASMReady(){

    }

    update(){

    }
}

export {BaseShapeJS}