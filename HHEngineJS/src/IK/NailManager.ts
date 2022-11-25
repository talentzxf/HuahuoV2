import {huahuoEngine} from "../EngineAPI";

declare var Module: any;
import {getMethodsAndVariables} from "hhcommoncomponents"
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {BaseSolidShape} from "../Shapes/BaseSolidShape";
import {NailShapeJS} from "../Shapes/NailShapeJS";

const eps: number = 0.001;

class NailManager {
    cppNailManager

    constructor() {
        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
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

    checkDuplication(shape1: BaseShapeJS, shape2: BaseShapeJS): boolean {
        return this.cppNailManager.CheckDuplication(shape1.getRawShape(), shape2.getRawShape())
    }

    shapeMoved(shape: BaseSolidShape, exceptShapes: Set<BaseShapeJS> = null, isTransformationPermanent: boolean = false) {
        if (exceptShapes == null) {
            exceptShapes = new Set<BaseShapeJS>()
        }

        return this._shapeMoved(shape, exceptShapes, isTransformationPermanent)
    }

    nailMoved(nail: NailShapeJS, exceptShapes: Set<BaseShapeJS> = null, isTransformationPermanent: boolean = false) {
        if (exceptShapes == null) {
            exceptShapes = new Set<BaseShapeJS>()
        }
        return this._nailMoved(nail, exceptShapes, isTransformationPermanent)
    }

    // TODO: Convert iterate to loop.
    _shapeMoved(shape: BaseSolidShape, exceptShapes: Set<BaseSolidShape>, isTransformationPermanent: boolean = false) {
        exceptShapes.add(shape)

        // Shape moved, get all it's nails and update.
        let nailComponent = shape.getComponentByTypeName("NailComponent")
        let nails = nailComponent["getNails"]()

        // Update all the nail position
        for (let nail of nails) {

            if (exceptShapes.has(nail)) {
                continue
            }

            // Move the nail to the destination position
            let localPosition = nail.getLocalPositionInShape(shape)
            let newGlobalPosition = shape.localToGlobal(localPosition)
            if (nail.position.getDistance(newGlobalPosition) > eps) {
                nail.isTransformationPermanent = isTransformationPermanent
                // nail.position = newGlobalPosition
                nail.setParentLocalPosition(newGlobalPosition, false, false)
                this._nailMoved(nail, exceptShapes, isTransformationPermanent)
                nail.update()
                nail.isTransformationPermanent = true
            }
        }
    }

    _nailMoved(nail: NailShapeJS, exceptShapes: Set<BaseSolidShape>, isTransformationPermanent: boolean = false) {
        exceptShapes.add(nail)

        let currentNailPosition = nail.position

        for (let shape of nail.getBoundShapes()) {

            if (exceptShapes.has(shape))
                continue

            // Move the nail to the destination position
            let nailLocalPosition = nail.getLocalPositionInShape(shape)
            let nailVector = nailLocalPosition.subtract(shape.localPivotPosition)
            let nailTheta = nailVector.angle

            let vector = currentNailPosition.subtract(shape.position)

            shape.isTransformationPermanent = isTransformationPermanent
            shape.setRotation(vector.angle - nailTheta, false, false)
            shape.updatePositionAndRotation()

            let afterNailPosition = shape.localToGlobal(nailLocalPosition)
            let nailOffset = currentNailPosition.subtract(afterNailPosition)

            shape.isTransformationPermanent = isTransformationPermanent
            shape.setParentLocalPosition(shape.position.add(nailOffset), false, false)
            shape.isTransformationPermanent = true
            shape.updatePositionAndRotation()

            this._shapeMoved(shape, exceptShapes, isTransformationPermanent)

        }
    }

    update(){
        for(let index = 0; index < this.cppNailManager.GetNailCount(); index++){
            let nailRawObj = this.cppNailManager.GetNail(index)
            let nailShape: NailShapeJS = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(nailRawObj)

            for(let shape of nailShape.getBoundShapes()){
                let localNailPosition = nailShape.getLocalPositionInShape(shape)
                let prevGlobalNailPosition = shape.localToGlobal(localNailPosition)
                if(localNailPosition.getDistance(prevGlobalNailPosition) >= eps){
                    this.shapeMoved(shape)
                }
            }
        }
    }
}

function getNailManager(): NailManager {
    let nailManager = window["NailManagerJS"]
    if (!nailManager) {
        nailManager = new NailManager()
        window["NailManagerJS"] = nailManager
    }

    return nailManager
}

export {getNailManager}