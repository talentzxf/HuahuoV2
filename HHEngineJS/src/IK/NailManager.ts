import {BaseSolidShape} from "../Shapes/BaseSolidShape";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

class Nail {
    nailManager: NailManager

    shapeLocalPointMap: Map<BaseSolidShape, paper.Point> = new Map<BaseSolidShape, paper.Point>()

    paperShape: paper.Path

    position: paper.Point

    constructor(nailManager: NailManager) {
        this.nailManager = nailManager
    }

    containShape(shape){
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
        return true
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

class NailManager {
    private nails: Set<Nail> = new Set<Nail>()

    private shapeNailMap: Map<BaseSolidShape, Set<Nail>> = new Map();
    private dirty: Boolean = false
    private lastDrawFrame: number = -1

    setDirty(dirtyFlag: boolean) {
        this.dirty = dirtyFlag
    }

    /**
     * Return true, means passed. The nail can be created because there's no duplication
     * Return false, means failed. The nail can't be created because there's already a nail with these two shapes.
     * @param shape1
     * @param shape2
     */
    checkDuplication(shape1: BaseSolidShape, shape2: BaseSolidShape) { // If two shapes has already been nailed together, the nail shouldn't be created.
        if(!this.shapeNailMap.has(shape1) || this.shapeNailMap.has(shape2)){
            return true
        }

        let shape1NailSet = this.shapeNailMap.get(shape1)
        for(let nail of shape1NailSet){
            if(nail.containShape(shape2))
                return false
        }
        return true
    }

    createNail() {
        let newNail = new Nail(this)
        this.nails.add(newNail)

        this.dirty = true
        return newNail
    }

    addNailShapeMapping(shape: BaseSolidShape, nail: Nail) {
        let nailSet = this.shapeNailMap.get(shape)
        if(nailSet == null){
            nailSet = new Set<Nail>()
            this.shapeNailMap.set(shape, nailSet)
        }

        nailSet.add(nail)
    }

    updateAllNails(frameId) {
        if (this.lastDrawFrame == -1 || this.dirty || this.lastDrawFrame != frameId) {
            for (let nail of this.nails) {
                nail.update()
            }

            this.dirty = false
            this.lastDrawFrame = frameId
        }
    }
}

let managerName = "NailManager"

function getNailManager(): NailManager {
    let nailManager = window[managerName]
    if (!nailManager) {
        nailManager = new NailManager()
        window[managerName] = nailManager
    }

    return nailManager
}

export {getNailManager, Nail}