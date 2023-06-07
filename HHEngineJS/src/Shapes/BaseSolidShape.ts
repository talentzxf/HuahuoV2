import {BaseShapeJS} from "./BaseShapeJS";
import {StrokeComponent} from "../Components/StrokeComponent";
import {FillColorComponent} from "../Components/FillColorComponent";

abstract class BaseSolidShape extends BaseShapeJS {
    constructor(rawObj?) {
        let needInitComponents = false
        if(!rawObj){
            needInitComponents = true
        }

        super(rawObj);

        if(needInitComponents){
            this.addComponent(new StrokeComponent())
            this.addComponent(new FillColorComponent())
        }

    }
}

export {BaseSolidShape}