import {Vector2} from "hhcommoncomponents"
import {BaseShapeJS} from "./BaseShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";

let shapeName = "ParticleSystem"

class ParticleSystemJS extends BaseShapeJS {
    static createParticleSystem(rawObj) {
        return new ParticleSystemJS(rawObj)
    }

    boundRectangle: paper.Path.Rectangle
    raster: paper.Raster


    getShapeName(): string {
        return shapeName
    }

    createShape() {
        super.createShape()

        let p1Global = this.getPaperPoint(this.rawObj.GetStartPoint())
        let p2Global = this.getPaperPoint(this.rawObj.GetEndPoint())
        let paperjs = this.getPaperJs()
        this.paperItem = new paperjs.Group()
        this.paperItem.applyMatrix = false
        this.paperItem.data.meta = this

        this.paperItem.position = p1Global.add(p2Global).divide(2.0)

        let p1 = this.globalToLocal(p1Global)
        let p2 = this.globalToLocal(p2Global)
        let width = p2.x - p1.x
        let height = p2.y - p1.y

        this.boundRectangle = new paperjs.Path.Rectangle(p1, p2)
        this.boundRectangle.applyMatrix = false;
        this.boundRectangle.strokeColor = new paperjs.Color("black");
        this.boundRectangle.fillColor = new paperjs.Color('#00000000') // Transparent
        this.boundRectangle.data.meta = this

        this.raster = new paperjs.Raster(new paper.Point(width, height))
        this.raster.applyMatrix = false;
        this.raster.strokeColor = new paperjs.Color("black");
        this.raster.fillColor = new paperjs.Color('#00000000') // Transparent
        this.raster.data.meta = this

        this.raster.position = p2.add(p1).divide(2.0)

        this.paperItem.addChild(this.boundRectangle)
        this.paperItem.addChild(this.raster)

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

    getLeftUp() {
        return this.getPaperPoint(this.rawObj.GetStartPoint())
    }

    getRightDown() {
        return this.getPaperPoint(this.rawObj.GetEndPoint())
    }

    setPixel(i, j, color) {
        this.raster.setPixel(i, j, color)
    }

    override isSegmentSeletable() {
        return false
    }
}

clzObjectFactory.RegisterClass(shapeName, ParticleSystemJS.createParticleSystem)
export {ParticleSystemJS}