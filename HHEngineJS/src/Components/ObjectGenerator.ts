import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {LoadShapeFromCppShape} from "../Shapes/LoadShape";
import {GraphAction} from "../EventGraph/GraphActions";

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

    // Objects generated through this method won't sync with original object.
    @GraphAction()
    generateObject(){
        let rawObj = this.targetShape.rawObj
        let duplicatedShape = LoadShapeFromCppShape(rawObj, false, false, true)
        duplicatedShape.isSelectable = function (){
            return false
        }

        duplicatedShape.setSelectedMeta(null)
        duplicatedShape.isMirage = true

        duplicatedShape.getAction().setPosition(this.baseShape.position.x, this.baseShape.position.y)

        duplicatedShape.update(true)
        this.generatedShapeArray.push(duplicatedShape)
    }

    cleanUp() {
        super.cleanUp();

        // TODO: Really delete these mirage objects?
        for(let generatedShape of this.generatedShapeArray){
            generatedShape.removePaperObj()
        }

        this.generatedShapeArray = new Array<BaseShapeJS>()
    }
}

export {ObjectGenerator}