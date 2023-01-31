import {Vector2} from "hhcommoncomponents"
import {BaseShapeJS} from "./BaseShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";

let shapeName = "ParticleSystem"
class ParticleSystemJS extends BaseShapeJS{
    static createParticleSystem(rawObj){
        return new ParticleSystemJS(rawObj)
    }

    getShapeName(): string {
        return shapeName
    }

    createShape(){
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

    setStartPoint(startPoint: Vector2){
        this.rawObj.SetStartPoint(startPoint.x, startPoint.y, 0)
    }

    setEndPoint(endPoint: Vector2){
        this.rawObj.SetEndPoint(endPoint.x, endPoint.y, 0);

        if(this.paperShape != null){
            this.paperShape.remove()
        }

        this.createShape()
        this.store()
    }
}

clzObjectFactory.RegisterClass(shapeName, ParticleSystemJS.createParticleSystem)
export {ParticleSystemJS}