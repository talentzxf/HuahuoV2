import {BaseShapeJS} from "./BaseShapeJS";
import {StrokeComponent} from "../Components/StrokeComponent";
import {FillColorComponent} from "../Components/FillColorComponent";

abstract class BaseSolidShape extends BaseShapeJS {
    lastRenderFrame = -1

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

    override update(force:boolean = false) {
        let currentFrame = this.getLayer().GetCurrentFrame()
        if(force || currentFrame != this.lastRenderFrame){
            super.update(true);

            this.lastRenderFrame = currentFrame
        }
    }
}

export {BaseSolidShape}