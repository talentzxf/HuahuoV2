import {huahuoEngine} from "../EngineAPI"

// This class and all it's children should not store any data
// All persistent data should be stored in C++'s side.
class BaseShapeJS{
    protected cppShapeObj

    constructor() {
        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            _this.cppShapeObj = huahuoEngine.CreateShape(_this.getShapeName())
        })
    }

    getShapeName(){
        return "Unkonwn"
    }

    getRawShape(){
        return this.cppShapeObj;
    }

    update(){

    }
}

export {BaseShapeJS}