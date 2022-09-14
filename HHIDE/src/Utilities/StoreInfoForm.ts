import {CustomElement} from "hhcommoncomponents";
import {renderEngine2D, Player, huahuoEngine} from "hhenginejs"
import {CSSDefines} from "./CSSDefines";
import {HHForm} from "./HHForm";

@CustomElement({
    selector: "hh-store-info"
})
class StoreInfoForm extends HTMLElement implements HHForm{
    form: HTMLFormElement
    formCloseBtn: HTMLElement
    previewSceneContainer: HTMLDivElement
    selector: string;

    previewCanvas: HTMLCanvasElement
    previewAnimationPlayer: Player

    onOKCallback: Function;

    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"

        this.innerHTML += CSSDefines.formStyle

        // Add title.
        this.innerHTML += "<form>" +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='formCloseBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "<h3>Store Info</h3>" +
            "</form>"

        this.form = this.querySelector("form")
        this.formCloseBtn = this.querySelector("#formCloseBtn")
        this.formCloseBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.previewSceneContainer = document.createElement("div")
        this.previewCanvas = document.createElement("canvas") as HTMLCanvasElement
        this.previewCanvas.style.border = "1px solid blue"
        this.previewSceneContainer.appendChild(this.previewCanvas)
        this.form.appendChild(this.previewSceneContainer)

        this.previewAnimationPlayer = new Player()
        renderEngine2D.init(this.previewCanvas)
        this.RedrawFrame(0)
    }

    closeForm() {
        this.style.display = "none"
    }

    RedrawFrame(frameId:number){
        let prevStore = huahuoEngine.GetCurrentStoreId()
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(0) // Only render the main store.
        let previousCanvas = renderEngine2D.setDefaultCanvas(this.previewCanvas)
        renderEngine2D.clearBackground()
        huahuoEngine.Set
        this.previewAnimationPlayer.setFrameId(frameId)
        if(previousCanvas)
            renderEngine2D.setDefaultCanvas(previousCanvas)

        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStore)
    }
}

export {StoreInfoForm}