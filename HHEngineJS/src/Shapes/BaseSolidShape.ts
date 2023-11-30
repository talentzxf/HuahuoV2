import {BaseShapeJS} from "./BaseShapeJS";
import {StrokeComponent} from "../Components/StrokeComponent";
import {FillColorComponent} from "../Components/FillColorComponent";
import {PropertyType} from "hhcommoncomponents";

abstract class BaseSolidShape extends BaseShapeJS {
    override initShapeFromEditor() {
        super.initShapeFromEditor();

        this.addComponent(new StrokeComponent())
        this.addComponent(new FillColorComponent())
    }

    afterWASMReady() {
        super.afterWASMReady();

        let segmentComponentProperty = {
            key: "Segment",
            type: PropertyType.COMPONENT,
            config: {
                children: []
            }
        }

        this.propertySheet.addProperty(segmentComponentProperty)
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);
    }
}

export {BaseSolidShape}