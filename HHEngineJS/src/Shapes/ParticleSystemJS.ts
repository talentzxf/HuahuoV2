import {Vector2} from "hhcommoncomponents"
import {BaseShapeJS} from "./BaseShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";

let shapeName = "ParticleSystem"

class ParticleSystemJS extends BaseShapeJS {
    static createParticleSystem(rawObj) {
        return new ParticleSystemJS(rawObj)
    }

    hasUpdatedToRaster: boolean = false
    

    getShapeName(): string {
        return shapeName
    }

    createShape() {
        super.createShape()

        let p1 = this.getPaperPoint(this.rawObj.GetStartPoint())
        let p2 = this.getPaperPoint(this.rawObj.GetEndPoint())

        let paperjs = this.getPaperJs()
        this.paperShape = new paperjs.Path.Rectangle(p1, p2)
        this.paperShape.applyMatrix = false;
        this.paperShape.strokeColor = new paperjs.Color("black");
        this.paperShape.fillColor = new paperjs.Color('#00000000') // Transparent
        this.paperShape.data.meta = this

        super.afterCreateShape()
    }

    setStartPoint(startPoint: Vector2) {
        this.rawObj.SetStartPoint(startPoint.x, startPoint.y, 0)
    }

    setEndPoint(endPoint: Vector2) {
        this.rawObj.SetEndPoint(endPoint.x, endPoint.y, 0);

        if (this.paperShape != null) {
            this.paperShape.remove()
        }

        this.createShape()
        this.store()
    }

    upgradeToRasterObj() {
        if (!this.hasUpdatedToRaster) {
            let p1 = this.getPaperPoint(this.rawObj.GetStartPoint())
            let p2 = this.getPaperPoint(this.rawObj.GetEndPoint())

            let paperjs = this.getPaperJs()
            this.paperItem = new paperjs.Raster(new paper.Point(p2.x - p1.x, p2.y - p1.y))
            this.paperItem.applyMatrix = false;
            this.paperItem.strokeColor = new paperjs.Color("black");
            this.paperItem.fillColor = new paperjs.Color('#00000000') // Transparent
            this.paperItem.data.meta = this

            this.hasUpdatedToRaster = true
        }
    }

    getLeftUp() {
        return this.getPaperPoint(this.rawObj.GetStartPoint())
    }

    getRightDown() {
        return this.getPaperPoint(this.rawObj.GetEndPoint())
    }

    setPixel(i, j, color) {
        // Change to raster object.
        this.upgradeToRasterObj();

        (this.paperItem as paper.Raster).setPixel(i, j, color)
    }

    override isSegmentSeletable() {
        return false
    }
}

clzObjectFactory.RegisterClass(shapeName, ParticleSystemJS.createParticleSystem)
export {ParticleSystemJS}