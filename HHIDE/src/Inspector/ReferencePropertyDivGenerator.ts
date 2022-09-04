import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";
import {BaseShapeJS} from "hhenginejs"
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {ShapePicker} from "../ShapeDrawers/ShapePicker";

class ReferencePropertyDesc extends BasePropertyDesc{

    shapePicker: ShapePicker
    referenceDiv: HTMLDivElement
    shapeNameSpan: HTMLSpanElement

    formatShapeName(type:string, name:string){
        return type + "[" + name + "]"
    }

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

        this.shapePicker.onShapePicked = this.onShapePicked.bind(this)

        this.referenceDiv = document.createElement("div")
        this.shapeNameSpan = document.createElement("span")
        this.shapeNameSpan.innerText = this.formatShapeName(type, name)
        this.referenceDiv.appendChild(this.shapeNameSpan)

        let shapeSelectButton = document.createElement("input")
        shapeSelectButton.type = "button"
        shapeSelectButton.value = i18n.t("inspector.Pickup")

        shapeSelectButton.addEventListener("click", this.beginToPickupShape.bind(this))
        this.referenceDiv.appendChild(shapeSelectButton)

        this.contentDiv.appendChild(this.referenceDiv)
    }

    onShapePicked(selectedShape:BaseShapeJS){
        let typeName = selectedShape.typename
        let name = selectedShape.name

        this.shapeNameSpan.innerText = this.formatShapeName(typeName, name)
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