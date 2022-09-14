import {CustomElement} from "hhcommoncomponents";
import {SceneView} from "../SceneView/SceneView";
import {CSSDefines} from "./CSSDefines";
import {HHForm} from "./HHForm";

@CustomElement({
    selector: "hh-store-info"
})
class StoreInfoForm extends HTMLElement implements HHForm{
    previewSceneContainer: HTMLDivElement
    previewSceneView: SceneView
    selector: string;

    onOKCallback: Function;

    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"

        this.innerHTML += CSSDefines.formStyle

        // Add title.
        this.innerHTML += "<h3>Store Info</h3>"

        this.previewSceneContainer = document.createElement("div")
        this.previewSceneView = document.createElement("hh-sceneview") as SceneView
        this.previewSceneContainer.appendChild(this.previewSceneView)

        this.appendChild(this.previewSceneContainer)
    }

    closeForm() {
        this.style.display = "none"
    }
}

export {StoreInfoForm}