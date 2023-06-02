import {BasePropertyDesc, BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";
import {BaseShapeJS} from "hhenginejs"
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {ShapePicker} from "../ShapeDrawers/ShapePicker";
import {HHToast} from "hhcommoncomponents";

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
            type = targetShape.typename
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
        if(this.property.setter(selectedShape)){
            this.onValueChanged(selectedShape)
        }else{
            HHToast.warn(i18n.t("toast.cantSelect"))
        }

        let currentFocusedSceneView = sceneViewManager.getFocusedSceneView()
        currentFocusedSceneView.resetDefaultShapeDrawer()
        currentFocusedSceneView.endOfDrawingShape(this.shapePicker)
    }

    onEntryAdded() {
        super.onEntryAdded();
        this.beginToPickupShape()
    }

    onValueChanged(val) {
        if(val == null)
            return;
        
        let typeName = val.typename
        let name = val.name

        this.shapeNameSpan.innerText = this.formatShapeName(typeName, name)
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