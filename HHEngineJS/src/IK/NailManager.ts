import {huahuoEngine} from "../EngineAPI";

declare var Module: any;
import {getMethodsAndVariables} from "hhcommoncomponents"
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {NailShapeJS} from "../Shapes/NailShapeJS";

const eps: number = 0.1;

const ITERATETHRESHOLD = 5;
const MAX_ITERATE = 2

/**
 * Use FABRIK to do the IK. http://andreasaristidou.com/FABRIK.html
 */
class NailManager {
    cppNailManager

    nailShape: paper.Path

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

    shapeMoved(shape: BaseShapeJS, isTransformationPermanent: boolean = false) {
        let exceptShapes = new Set<BaseShapeJS>()
        let tracePath = new Array<BaseShapeJS>()

        let iteratedCount = new Map<NailShapeJS, number>()
        this._shapeMoved(shape, exceptShapes, tracePath, isTransformationPermanent, iteratedCount)
    }

    nailMoved(nail: NailShapeJS, isTransformationPermanent: boolean = false) {

        console.log("Nail Target:" + nail.position)
        if (this.nailShape == null) {
            this.nailShape = new paper.Path.Circle(nail.position, 20)
            this.nailShape.fillColor = new paper.Color("purple")
            this.nailShape.bringToFront()
        } else {
            this.nailShape.bringToFront()
            this.nailShape.position = nail.position
        }

        let targetPosition = nail.position
        let iterateCount = MAX_ITERATE

        let lastDistance = Number.NaN

        while (iterateCount > 0) {
            let exceptShapes = new Set<BaseShapeJS>()
            let tracePath = new Array<BaseShapeJS>();

            let iteratedCount = new Map<NailShapeJS, number>()

            // Iterate until target difference doesn't change or within a margin.
            this._nailMoved(nail, exceptShapes, tracePath, isTransformationPermanent, iteratedCount)

            let currentDistance = nail.position.getDistance(targetPosition)
            if (currentDistance < eps) {
                break;
            }

            if (Math.abs(currentDistance - lastDistance) < eps)
                break;

            lastDistance = currentDistance
            nail.isTransformationPermanent = isTransformationPermanent
            nail.setParentLocalPosition(targetPosition, false, false)
            nail.isTransformationPermanent = true
            iterateCount--
        }

        console.log("IK iterated:" + (MAX_ITERATE - iterateCount + 1) + " times")
    }

    // TODO: Convert iterate to loop.
    _shapeMoved(shape: BaseShapeJS, exceptShapes: Set<BaseShapeJS>, tracePath: Array<BaseShapeJS>,
                isTransformationPermanent, staticNailIteratedCount: Map<NailShapeJS, number>) {
        exceptShapes.add(shape)

        tracePath.push(shape)

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
                if (!nail.isStatic) { // The nail is actually static, set it's position back and back propograte.
                    nail.isTransformationPermanent = isTransformationPermanent
                    // nail.position = newGlobalPosition
                    nail.setParentLocalPosition(newGlobalPosition, false, false, false)
                    this._nailMoved(nail, exceptShapes, tracePath, isTransformationPermanent, staticNailIteratedCount)
                } else { // The nail is static, begin to back propagate. This time perform a REAL FABRIK algorithm
                    tracePath.push(nail) // Add the nail into path.
                    this.realFABRIK(tracePath, isTransformationPermanent, staticNailIteratedCount)
                }

                nail.update()
                nail.isTransformationPermanent = true
            }
        }

        tracePath.pop()
    }

    _nailMoved(nail: NailShapeJS, exceptShapes: Set<BaseShapeJS>, tracePath: Array<BaseShapeJS>,
               isTransformationPermanent, staticNailIteratedCount: Map<NailShapeJS, number>) {
        exceptShapes.add(nail)
        tracePath.push(nail)

        for (let shape of nail.getBoundShapes()) {
            if (exceptShapes.has(shape))
                continue

            this.moveShapeBasedOnNailMovement(nail, shape, null, isTransformationPermanent)
            this._shapeMoved(shape, exceptShapes, tracePath, isTransformationPermanent, staticNailIteratedCount)
        }

        tracePath.pop()
    }

    moveShapeBasedOnNailMovement(nail: NailShapeJS, shape: BaseShapeJS, targetNail: NailShapeJS, isTransformationPermanent: boolean) {
        let currentNailPosition = nail.position
        let nailLocalPosition = nail.getLocalPositionInShape(shape)

        let vector = null
        if (targetNail) { // The difference between a fake FABRIK and a real FABRIK
            vector = currentNailPosition.subtract(targetNail.position)
        } else {
            vector = currentNailPosition.subtract(shape.position)
        }

        shape.isTransformationPermanent = isTransformationPermanent
        let nailVector = nailLocalPosition.subtract(shape.localPivotPosition)
        let nailTheta = nailVector.angle

        shape.setRotation(vector.angle - nailTheta, false, false)

        shape.updatePositionAndRotation()

        let afterNailPosition = shape.localToGlobal(nailLocalPosition)
        let nailOffset = currentNailPosition.subtract(afterNailPosition)

        shape.isTransformationPermanent = isTransformationPermanent
        shape.setParentLocalPosition(shape.position.add(nailOffset), false, false, false)
        shape.isTransformationPermanent = true
        shape.updatePositionAndRotation()
    }

    // TODO: What if there's a loop?? What if there are two nails fixed in the path?
    realFABRIK(path: Array<BaseShapeJS>, isTransformPermanent: boolean, staticNailIteratedCount: Map<NailShapeJS, number>) {
        let staticNail = path[path.length - 1] as NailShapeJS

        let staticNailPosition = staticNail.position
        if (staticNailIteratedCount.has(staticNail)) {
            staticNailIteratedCount.set(staticNail, staticNailIteratedCount.get(staticNail) + 1)
        } else {
            staticNailIteratedCount.set(staticNail, 0)
        }

        if (staticNailIteratedCount.get(staticNail) > ITERATETHRESHOLD) {
            return
        }

        // If we reached here. It means we hit a static nail.
        let involvedNails = new Set<NailShapeJS>()
        let involvedShapes = new Set<BaseShapeJS>()
        let exceptShapes = new Set<BaseShapeJS>()

        let stabledShapes = new Set<BaseShapeJS>()


        // TODO: Forward phase and back phase is almost the same. Merge them later.
        // Forward phase
        let currentIndex = 0
        if(!(path[0] instanceof NailShapeJS)){
            currentIndex = 1
        }
        while (currentIndex < path.length - 1) {
            let currentNail = path[currentIndex++] as NailShapeJS
            exceptShapes.add(currentNail)
            let currentShape = path[currentIndex++]
            if(currentShape == null)
                break;
            exceptShapes.add(currentShape)
            let nextNail = path[currentIndex] as NailShapeJS
            if (nextNail) {
                exceptShapes.add(nextNail)
            }
            this.moveShapeBasedOnNailMovement(currentNail, currentShape, nextNail, isTransformPermanent)

            if (nextNail) {
                // Move the nail to the destination position
                let localPosition = nextNail.getLocalPositionInShape(currentShape)
                let newGlobalPosition = currentShape.localToGlobal(localPosition)
                nextNail.isTransformationPermanent = isTransformPermanent
                nextNail.setParentLocalPosition(newGlobalPosition, false, false)
            }
        }

        staticNail.setParentLocalPosition(staticNailPosition, false, false)
        exceptShapes.clear()
        // Backward phase
        currentIndex = path.length - 1
        while (currentIndex >= 1) { // We need to decrease the index twice in the loop. So currentIndex need to be larger than 1 to enter the loop.
            let currentNail = path[currentIndex--] as NailShapeJS
            exceptShapes.add(currentNail)
            involvedNails.add(currentNail)

            let currentShape = path[currentIndex--]
            exceptShapes.add(currentShape)
            involvedShapes.add(currentShape)

            let nextNail = path[currentIndex] as NailShapeJS // nextNail doesn't need to decrease index, cause it will be the next currentNail.
            if (nextNail) {
                exceptShapes.add(nextNail)
                involvedNails.add(nextNail)
            }

            let prevShapePosition = currentShape.position
            let prevShapeRotation = currentShape.rotation
            this.moveShapeBasedOnNailMovement(currentNail, currentShape, nextNail, isTransformPermanent)

            if (prevShapePosition.getDistance(currentShape.position) < eps && Math.abs(prevShapeRotation - currentShape.rotation) < eps) {
                stabledShapes.add(currentShape)
            }

            if (nextNail) {
                // Move the nail to the destination position
                let localPosition = nextNail.getLocalPositionInShape(currentShape)
                let newGlobalPosition = currentShape.localToGlobal(localPosition)
                nextNail.isTransformationPermanent = isTransformPermanent

                // The nail doesn't change much, remove it from involvedNails.
                if (newGlobalPosition.getDistance(nextNail.position) < eps) {
                    stabledShapes.add(nextNail)
                }else{
                    // nail.position = newGlobalPosition
                    nextNail.setParentLocalPosition(newGlobalPosition, false, false, false)
                }
            }
        }

        // Adjust all nails if they are involved.
        for (let nail of involvedNails) {
            if (stabledShapes.has(nail))
                continue
            let newTracePath = new Array()
            this._nailMoved(nail, exceptShapes, newTracePath, isTransformPermanent, staticNailIteratedCount)
        }

        for (let shape of involvedShapes) {
            if (stabledShapes.has(shape))
                continue

            let newTracePath = new Array()
            this._shapeMoved(shape, exceptShapes, newTracePath, isTransformPermanent, staticNailIteratedCount)
        }
    }

    update() {
        for (let index = 0; index < this.cppNailManager.GetNailCount(); index++) {
            let nailRawObj = this.cppNailManager.GetNail(index)
            let nailShape: NailShapeJS = huahuoEngine.getActivePlayer().getJSShapeFromRawShape(nailRawObj)

            for (let shape of nailShape.getBoundShapes()) {
                let localNailPosition = nailShape.getLocalPositionInShape(shape)
                let prevGlobalNailPosition = shape.localToGlobal(localNailPosition)

                if (nailShape.position.getDistance(prevGlobalNailPosition) >= eps) {
                    this.nailMoved(nailShape) // TODO: Nail moved or shape moved??
                    this.shapeMoved(shape)
                }
            }
        }
    }
}

export {NailManager}