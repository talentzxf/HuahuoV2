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
        this.previewSceneContainer.style.width = "300px"
        this.previewSceneContainer.style.height = "300px"
        this.previewCanvas = document.createElement("canvas") as HTMLCanvasElement
        this.previewCanvas.style.border = "1px solid blue"
        this.previewSceneContainer.appendChild(this.previewCanvas)
        this.form.appendChild(this.previewSceneContainer)

        this.previewAnimationPlayer = new Player()
        renderEngine2D.init(this.previewCanvas)

        let [initW, initH] = renderEngine2D.getInitCanvasWH()

        if(initW > 0){
            renderEngine2D.resize(this.previewCanvas, initW, initH)
        }

        window.addEventListener("resize", this.OnResize.bind(this))

        let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
        resizeObserver.observe(this.previewSceneContainer)
    }

    closeForm() {
        this.style.display = "none"
    }

    OnResize(){
        if(window.getComputedStyle(this.parentElement).display == "none")
            return;

        let containerWidth = this.previewSceneContainer.clientWidth
        let containerHeight = this.previewSceneContainer.clientHeight
        let margin = 15
        let proposedCanvasWidth = containerWidth - margin
        let proposedCanvasHeight = containerHeight - margin

        let [actualCanvasWidth, actualCanvasHeight] = renderEngine2D.getContentWH(proposedCanvasWidth, proposedCanvasHeight)

        renderEngine2D.resize(this.previewCanvas, actualCanvasWidth, actualCanvasHeight)

        this.previewCanvas.width = actualCanvasWidth
        this.previewCanvas.height = actualCanvasHeight
        this.previewCanvas.style.width = actualCanvasWidth + "px"
        this.previewCanvas.style.height = actualCanvasHeight + "px"
        this.previewCanvas.style.position = "relative"
        this.previewCanvas.style.left = (containerWidth - actualCanvasWidth) / 2 + "px"
        this.previewCanvas.style.top = (containerHeight - actualCanvasHeight) / 2 + "px"
        this.RedrawFrame(0)
    }

    RedrawFrame(frameId:number){
        let prevStore = huahuoEngine.GetCurrentStoreId()
        let layer = huahuoEngine.GetCurrentLayer()
        let prevFrameId = layer.GetCurrentFrame()

        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(1) // Only render the main store.i.e. the 1st store.
        let previousCanvas = renderEngine2D.setDefaultCanvas(this.previewCanvas)
        renderEngine2D.clearBackground()
        this.previewAnimationPlayer.loadShapesFromStore()
        this.previewAnimationPlayer.setFrameId(frameId)
        if(previousCanvas)
            renderEngine2D.setDefaultCanvas(previousCanvas)

        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStore)
        this.previewAnimationPlayer.setFrameId(prevFrameId)
    }
}

export {StoreInfoForm}