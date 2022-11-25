import * as paper from "paper";
import {Vector2} from "hhcommoncomponents"
import {MirrorComponent} from "../Components/MirrorComponent";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {BaseSolidShape} from "./BaseSolidShape";

let shapeName = "MirrorShape"
class MirrorShapeJS extends BaseSolidShape{
    static createMirror(rawObj){
        return new MirrorShapeJS(rawObj)
    }

    randomStrokeColor: paper.Color

    mirrorComponent: MirrorComponent

    constructor(rawObj) {
        let needInitComponents = false
        if(!rawObj){
            needInitComponents = true
        }

        super(rawObj);

        if(needInitComponents){
            this.mirrorComponent = new MirrorComponent()
            this.addComponent(this.mirrorComponent)
        }
    }

    getShapeName(): string {
        return shapeName
    }

    createShape(){
        super.createShape()

        let p1 = this.getPaperPoint(this.rawObj.GetStartPoint())
        let p2 = this.getPaperPoint(this.rawObj.GetEndPoint())

        let paperjs = this.getPaperJs()

        this.paperShape = new paperjs.Path.Line( p1, p2)
        this.paperShape.applyMatrix = false;
        this.paperShape.strokeColor = this.randomStrokeColor;
        this.paperShape.data.meta = this

        super.afterCreateShape()
    }

    setStartPoint(startPoint: Vector2){
        this.rawObj.SetStartPoint(startPoint.x, startPoint.y, 0);
    }

    setEndPoint(endPoint: Vector2){
        this.rawObj.SetEndPoint(endPoint.x, endPoint.y, 0);

        if(this.paperShape == null){
            this.randomStrokeColor = paper.Color.random()
        } else {
            this.paperShape.remove()
        }

        this.createShape()
        this.store()
    }

    override hitTypeSelectable(hitType): boolean {
        if(hitType == "stroke")
            return false
        return true
    }
}

clzObjectFactory.RegisterClass(shapeName, MirrorShapeJS.createMirror)

export {MirrorShapeJS}