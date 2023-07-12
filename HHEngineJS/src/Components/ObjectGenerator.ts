import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {LoadShapeFromCppShape} from "../Shapes/LoadShape";
import {GraphAction} from "../EventGraph/GraphActions";
import {eventBus} from "hhcommoncomponents";
import {huahuoEngine} from "../EngineAPI";

@Component()
class ObjectGenerator extends AbstractComponent {
    @PropertyValue(PropertyCategory.shape)
    targetShape

    paperShapeGroup: paper.Group

    generatedShapeArray:Array<any> = new Array<BaseShapeJS>()

    animationStoppedEventHandler: number = -1
    constructor(rawObj?, isMirage = false) {
        super(rawObj, isMirage);

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false

        this.animationStoppedEventHandler = eventBus.addEventHandler("Player", "stopPlay", this.animationStopped.bind(this))
    }

    // Objects generated through this method won't sync with original object.
    @GraphAction(true)
    generateObject(){
        if(this.targetShape == null)
            return

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

    animationStopped(){
        this.removeAllMirateObjects()
    }

    removeAllMirateObjects(){
        // TODO: Really delete these mirage objects?
        for(let generatedShape of this.generatedShapeArray){
            generatedShape.removePaperObj()
        }

        this.generatedShapeArray = new Array<BaseShapeJS>()
    }

    cleanUp() {
        super.cleanUp();

        this.removeAllMirateObjects()

        if(this.animationStoppedEventHandler >= 0){
            eventBus.removeEventHandler("Player", "stopPlay", this.animationStoppedEventHandler)
            this.animationStoppedEventHandler = -1
        }
    }
}

export {ObjectGenerator}