import {BaseSolidShape} from "../Shapes/BaseSolidShape";

class Nail{
    nailManager: NailManager

    shapeLocalPointMap: Map<BaseSolidShape, paper.Point> = new Map<BaseSolidShape, paper.Point>()

    paperShape: paper.Path

    position: paper.Point

    constructor(nailManager: NailManager) {
        this.nailManager = nailManager
    }

    // The point is in global world space.
    addShape(shape: BaseSolidShape, point:paper.Point){
        this.position = point

        let localPoint = shape.globalToLocal(point)
        this.shapeLocalPointMap.set(shape, localPoint)

        this.nailManager.setDirty(true)
    }

    update(){
        if(this.position == null)
            return;

        if(this.paperShape == null){
            this.paperShape = new paper.Path.RegularPolygon(this.position, 6, 20)
            this.paperShape.fillColor = new paper.Color(0.71, 0.25, 0.4, 1.0)
            this.paperShape.strokeColor = new paper.Color("black")
        }

        this.paperShape.position = this.position
        this.paperShape.bringToFront()
    }
}

class NailManager{
    private nails: Array<Nail> = new Array<Nail>()

    private dirty: Boolean = false
    private lastDrawFrame: number = -1

    setDirty(dirtyFlag: boolean){
        this.dirty = dirtyFlag
    }

    createNail(){
        let newNail = new Nail(this)
        this.nails.push(newNail)

        this.dirty = true
        return newNail
    }

    updateAllNails(frameId){
        if(this.lastDrawFrame == -1 || this.dirty || this.lastDrawFrame != frameId){
            for(let nail of this.nails){
                nail.update()
            }

            this.dirty = false
            this.lastDrawFrame = frameId
        }
    }
}

let managerName = "NailManager"
function getNailManager():NailManager{
    let nailManager = window[managerName]
    if(!nailManager){
        nailManager = new NailManager()
        window[managerName] = nailManager
    }

    return nailManager
}

export {getNailManager, Nail}