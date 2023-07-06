import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

@Component()
class ObjectGenerator extends AbstractComponent {
    @PropertyValue(PropertyCategory.shape)
    targetShape

    paperShapeGroup: paper.Group

    generatedShapeArray:Array<any> = new Array<BaseShapeJS>()

    constructor(rawObj?, isMirage = false) {
        super(rawObj, isMirage);

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
    }

    cleanUp() {
        super.cleanUp();

        for(let generatedShapeArray of this.generatedShapeArray){
            for(let mirageShape of )
        }
    }
}