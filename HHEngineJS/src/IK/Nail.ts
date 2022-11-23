import {BaseSolidShape} from "../Shapes/BaseSolidShape";
import {NailManager} from "./NailManager";
import {Logger} from "hhcommoncomponents";

class Nail {
    nailManager: NailManager

    shapeLocalPointMap: Map<BaseSolidShape, paper.Point> = new Map<BaseSolidShape, paper.Point>()

    paperShape: paper.Path

    position: paper.Point

    constructor(nailManager: NailManager) {
        this.nailManager = nailManager
    }

    getShapes() {
        return this.shapeLocalPointMap.keys()
    }

    containShape(shape) {
        return this.shapeLocalPointMap.has(shape)
    }

    // The point is in global world space.
    addShape(targetShape: BaseSolidShape, point: paper.Point): Boolean {
        for (let [shape, point] of this.shapeLocalPointMap) {
            if (!this.nailManager.checkDuplication(shape, targetShape)) { // These two shapes has already been nailed together. Not a valid nail.
                return false
            }
        }

        this.position = point

        let localPoint = targetShape.globalToLocal(point)
        this.shapeLocalPointMap.set(targetShape, localPoint)

        this.nailManager.setDirty(true)

        this.nailManager.addNailShapeMapping(targetShape, this)

        let _this = this
        // Only register for affine transforms
        targetShape.registerValueChangeHandler("position|scaling|rotation")(() => {
            _this.shapeMoved(targetShape)

            for (let [shape, localPoint] of this.shapeLocalPointMap) {
                if (shape != targetShape) // No need to update the same shape again.
                {
                    shape.update(true)
                }
            }
        })

        return true
    }

    shapeMoved(shape: BaseSolidShape) {
        let localPoint = this.shapeLocalPointMap.get(shape)
        if (localPoint == null) {
            Logger.error("Why local point is null???")
            return
        }

        this.position = shape.localToGlobal(localPoint)
        this.update()
    }

    getNailLocalLocation(shape: BaseSolidShape) {
        return this.shapeLocalPointMap.get(shape)
    }

    update() {
        if (this.position == null)
            return;

        if (this.paperShape == null) {
            this.paperShape = new paper.Path.RegularPolygon(this.position, 6, 20)
            this.paperShape.fillColor = new paper.Color(0.71, 0.25, 0.4, 1.0)
            this.paperShape.strokeColor = new paper.Color("black")
        }

        this.paperShape.position = this.position
        this.paperShape.bringToFront()
    }
}

export {Nail}