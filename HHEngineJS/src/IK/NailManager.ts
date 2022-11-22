import {BaseSolidShape} from "../Shapes/BaseSolidShape";
import {Nail} from "./Nail";


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
        if(!this.shapeNailMap.has(shape1) || !this.shapeNailMap.has(shape2)){
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
        this.dirty = true

        let nailSet = this.shapeNailMap.get(shape)
        if(nailSet == null){
            nailSet = new Set<Nail>()
            this.shapeNailMap.set(shape, nailSet)
        }

        nailSet.add(nail)
    }

    removeNail(nail:Nail){
        this.dirty = true

        this.nails.delete(nail)

        let involvedShapes = nail.getShapes()
        for(let shape of involvedShapes){
            if(this.shapeNailMap.has(shape)){
                let nails = this.shapeNailMap.get(shape)
                nails.delete(nail)
                this.shapeNailMap.set(shape, nails)
            }
        }
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

export {getNailManager, NailManager}