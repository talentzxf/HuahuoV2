import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {ShapePicker} from "../ShapeDrawers/ShapePicker";

class ReferencePropertyDesc extends BasePropertyDesc{

    shapePicker: ShapePicker
    referenceDiv: HTMLDivElement
    constructor(property: Property) {
        super(property);

        let targetShape = property.getter()

        let type = i18n.t("inspector.Unknown")
        let name = i18n.t("inspector.Unknown")

        if(targetShape){
            type = targetShape.typeName
            name = targetShape.name
        }

        this.shapePicker = new ShapePicker()

        this.referenceDiv = document.createElement("div")
        let shapeNameSpan = document.createElement("span")
        shapeNameSpan.innerText = type + "[" + name + "]"
        this.referenceDiv.appendChild(shapeNameSpan)

        let shapeSelectButton = document.createElement("input")
        shapeSelectButton.type = "button"
        shapeSelectButton.value = i18n.t("inspector.Pickup")

        shapeSelectButton.addEventListener("click", this.beginToPickupShape.bind(this))
        this.referenceDiv.appendChild(shapeSelectButton)

        this.contentDiv.appendChild(this.referenceDiv)
    }

    onValueChanged(val) {
    }

    beginToPickupShape(){
        let currentFocusedSceneView = sceneViewManager.getFocusedSceneView()
        document.body.style.cursor = "pointer"
        currentFocusedSceneView.beginToDrawShape(this.shapePicker)
    }
}

class ReferencePropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new ReferencePropertyDesc(property);
    }
}

let referencePropertyDivGenerator = new ReferencePropertyDivGenerator()
export {referencePropertyDivGenerator}