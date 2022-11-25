import {huahuoEngine} from "../EngineAPI";

declare var Module: any;
import {getMethodsAndVariables} from "hhcommoncomponents"
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {BaseSolidShape} from "../Shapes/BaseSolidShape";
import {NailShapeJS} from "../Shapes/NailShapeJS";

const eps: number = 0.1;

/**
 * Use FABRIK to do the IK. http://andreasaristidou.com/FABRIK.html
 */
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

    shapeMoved(shape: BaseSolidShape, isTransformationPermanent: boolean = false) {
        let exceptShapes = new Set<BaseShapeJS>()

        this._shapeMoved(shape, exceptShapes, isTransformationPermanent)
    }

    nailMoved(nail: NailShapeJS, isTransformationPermanent: boolean = false) {
        let targetPosition = nail.position
        let lastDifference = Number.NaN
        let difference = Number.NaN

        let firstRun = true

        let totallyIterated = 0

        do{
            let exceptShapes = new Set<BaseShapeJS>()

            if(!firstRun){
                lastDifference = difference
            }

            nail.isTransformationPermanent = isTransformationPermanent
            nail.setParentLocalPosition(targetPosition, false, false)
            // Iterate until target difference doesn't change or within a margin.
            this._nailMoved(nail, exceptShapes, isTransformationPermanent)
            difference = targetPosition.getDistance(nail.position)
            firstRun = false

            if(Math.abs(lastDifference - difference) <= eps){ // If the residue error doesn't change.
                break;
            }
            totallyIterated++
        }while(Math.abs(difference) > eps )

        console.log("Totally iterated:" + totallyIterated + " times of IK.")

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
            let prevNailPosition = nail.position
            if (prevNailPosition.getDistance(newGlobalPosition) > eps) {
                if(!nail.isStatic) { // The nail is actually static, set it's position back and back propograte.
                    nail.isTransformationPermanent = isTransformationPermanent
                    // nail.position = newGlobalPosition
                    nail.setParentLocalPosition(newGlobalPosition, false, false)
                    this._nailMoved(nail, exceptShapes, isTransformationPermanent)
                }else{ // The nail is static, begin to back propagate.
                    let bpExcludedShapes = new Set<BaseShapeJS>()
                    this._nailMoved(nail, bpExcludedShapes, isTransformationPermanent)
                }

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
                    this.nailMoved(nailShape) // TODO: Nail moved or shape moved??
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