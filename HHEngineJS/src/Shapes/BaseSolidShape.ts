import {BaseShapeJS} from "./BaseShapeJS";
import {StrokeComponent} from "../Components/StrokeComponent";
import {FillColorComponent} from "../Components/FillColorComponent";

abstract class BaseSolidShape extends BaseShapeJS {
    override initShapeFromEditor() {
        super.initShapeFromEditor();

        this.addComponent(new StrokeComponent())
        this.addComponent(new FillColorComponent())
        // this.addComponent(new SegmentComponent())
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);
    }
}

export {BaseSolidShape}