import {BaseShapeJS} from "./BaseShapeJS";
declare function castObject(obj:any, clz:any): any;
declare var Module: any;

let shapeName = "CircleShape"
class CircleShapeJS extends BaseShapeJS{
    cirle: paper.Path.Circle

    static createShape(rawObj){
        return new CircleShapeJS(rawObj);
    }

    afterWASMReady(){
        this.rawObj = castObject(this.rawObj, Module.CircleShape);
    }

    awakeFromLoad(){
        this.update();
    }

    getShapeName(){
        return shapeName
    }

    update(){
        let circleCenter = this.getPaperPoint(this.rawObj.GetCenter());
        let radius = this.rawObj.GetRadius();
        if(this.cirle == null){
            this.cirle = new paper.Path.Circle();
        }
    }
}
