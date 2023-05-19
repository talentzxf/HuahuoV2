import {BaseSolidShape} from "./BaseSolidShape";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {StarMirrorComponent} from "../Components/StarMirrorComponent";

let shapeName = "StarMirrorShape"
class StarMirrorShapeJS extends BaseSolidShape{
    starMirrorComponent: StarMirrorComponent

    static createStarMirror(rawObj){
        return new StarMirrorShapeJS(rawObj)
    }

    constructor(rawObj) {
        let needInitComponents = rawObj == null?true:false;
        super(rawObj);

        if(needInitComponents){
            this.starMirrorComponent = new StarMirrorComponent(this)
            this.addComponent(this.starMirrorComponent)
        }
    }

    isSegmentSeletable(){
        return false
    }

    getShapeName(): string {
        return shapeName
    }

    createShape(){
        super.createShape()

        let paperjs = this.getPaperJs()
        this.paperShape = new paper.Path.Star(new paper.Point(0,0), 12, 25, 40)
        this.paperShape.applyMatrix = false;
        this.paperShape.strokeColor = new paperjs.Color("black");
        this.paperShape.data.meta = this
    }
}

clzObjectFactory.RegisterClass(shapeName, StarMirrorShapeJS.createStarMirror)
export {StarMirrorShapeJS}