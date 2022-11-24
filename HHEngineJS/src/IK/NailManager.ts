import {huahuoEngine} from "../EngineAPI";
declare var Module: any;
import {getMethodsAndVariables} from "hhcommoncomponents"
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

class NailManager{
    cppNailManager
    constructor() {
        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            _this.cppNailManager = Module.NailManager.prototype.GetNailManager();
            _this.interceptFunctions()
        })
    }

    interceptFunctions() {
        // Proxy all paperGroup functions/methods.
        let _this = this
        getMethodsAndVariables(this.cppNailManager).forEach(key => {
            if (key == "constructor")
                return
            const originalProp = this.cppNailManager[key]

            if ("function" === typeof originalProp) {
                _this[key] = (...args) => {
                    return Reflect.apply(originalProp, _this.cppNailManager, args)
                }
            }
            // else {
            //     Object.defineProperty(_this, key, {
            //         get: function () {
            //             return _this.cppNailManager[key]
            //         },
            //         set: function (val) {
            //             _this.cppNailManager[key] = val
            //         }
            //     })
            // }
        })
    }

    checkDuplication(shape1: BaseShapeJS, shape2: BaseShapeJS):boolean{
        return this.cppNailManager.CheckDuplication(shape1.getRawShape(), shape2.getRawShape())
    }
}

function getNailManager(): NailManager{
    let nailManager = window["NailManagerJS"]
    if(!nailManager){
        nailManager = new NailManager()
        window["NailManagerJS"] = nailManager
    }

    return nailManager
}

export {getNailManager}