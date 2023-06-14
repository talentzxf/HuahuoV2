import {CustomElement} from "hhcommoncomponents";
import {renderEngine2D, Player, huahuoEngine} from "hhenginejs"
import {CSSUtils} from "../Utilities/CSSUtils";
import {HHForm} from "../Utilities/HHForm";
import {projectInfo} from "../SceneView/ProjectInfo";
import {SVGFiles} from "../Utilities/Svgs";
import {SnapshotUtils} from "../Utilities/SnapshotUtils";
import {api} from "../RESTApis/RestApi";
import {SceneView} from "../SceneView/SceneView";
import {BaseForm} from "./BaseForm";

@CustomElement({
    selector: "hh-project-info"
})
class ProjectInfoForm extends BaseForm{
    projectNameInput: HTMLInputElement
    projectDescriptionInput: HTMLTextAreaElement
    previewSceneContainer: HTMLDivElement
    okBtn: HTMLButtonElement

    previewCanvas: HTMLCanvasElement
    previewAnimationPlayer: Player

    onOKCallback: Function;

    projectNameCheckImg:HTMLImageElement
    okImg: string = SVGFiles.okImg
    notOkImg: string = SVGFiles.notOKImg

    _isNameValid: boolean = false

    set isNameValid(val:boolean){
        this._isNameValid = val
        if(val){
            this.projectNameCheckImg.src = this.okImg
        }else{
            this.projectNameCheckImg.src = this.notOkImg
        }
    }

    connectedCallback(){
        super.connectedCallback()

        let formDiv = document.createElement("div")

        this.modalTitle.innerText = "Project Info"

        // Add title.
        formDiv.innerHTML +=
            "   <label for='projectname' class='form-label'><b>ProjectName</b></label>" +
            "   <div style='display: flex; align-items: center'>" +
            "       <input type='text' class='form-control' placeholder='Enter Storename' id='projectname'> " +
            "       <img id='projectNameCheckImg' style='width:20px; height:20px'> " +
            "   </div>" +
            "   <label for='description' class='form-label'><b>Descripition</b></label>" +
            "   <textarea type='text' class='form-control' placeholder='Enter Description' id='projectdescription'> </textarea>" +
            "   <label for='preview' class='form-label'><b>Preview</b></label>" +
            "   <div id='projectinfo-canvas-container' style='width:300px; height: 200px'>" +
            "       <canvas id='projectinfo-preview-canvas' style='border: 1px solid blue'></canvas>" +
            "   </div>"

        this.okBtn = document.createElement("button")
        this.okBtn.className = "btn btn-primary"
        this.okBtn.innerText = "OK"
        this.okBtn.onclick = this.onOK.bind(this)
        this.footer.appendChild(this.okBtn)

        this.form.appendChild(formDiv)

        this.projectNameCheckImg = this.querySelector("#projectNameCheckImg")
        this.projectNameCheckImg.src = this.notOkImg

        this.projectDescriptionInput = this.querySelector("#projectdescription")

        this.form = this.querySelector("form")

        this.projectNameInput = this.querySelector("#projectname")
        let _this = this
        this.projectNameInput.addEventListener("input", (evt)=>{
            let candidateName = _this.projectNameInput.value
            if(_this.validateText(candidateName)){

                api.checkFileNameExistence(candidateName).then((result) => {
                    if(result && result.data && result.data["exist"] == false){ // The project name doesn't exist in the system now. Name is valid
                        _this.isNameValid = true
                    }else{
                        _this.isNameValid = false
                    }
                })
            }else{
                _this.isNameValid = false
            }
        })

        this.previewSceneContainer = this.querySelector("#projectinfo-canvas-container")
        this.previewCanvas = this.querySelector("#projectinfo-preview-canvas")

        this.previewAnimationPlayer = new Player()

        let prevCanvas = renderEngine2D.getDefaultCanvas()
        renderEngine2D.init(this.previewCanvas)

        if(prevCanvas) // Restore the previous canvas as default. Or else functionality will be broken.
            renderEngine2D.setDefaultCanvas(prevCanvas)

        let [initW, initH] = renderEngine2D.getInitCanvasWH()

        if(initW > 0){
            renderEngine2D.resize(this.previewCanvas, initW, initH)
        }

        window.addEventListener("resize", this.OnResize.bind(this))

        let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
        resizeObserver.observe(this.previewSceneContainer)
    }

    validateText(val: string):boolean{
        return /^[a-zA-Z0-9_-]+$/.test(val)
    }

    onOK(evt){
        evt.stopPropagation()
        evt.preventDefault()

        let projectName = this.projectNameInput.value
        if(!this.validateText(projectName)){
            return;
        }

        let coverPageBinary = SnapshotUtils.takeSnapshot(this.previewCanvas)
        projectInfo.Setup(projectName, this.projectDescriptionInput.value, coverPageBinary)

        if(this.onOKCallback){
            this.onOKCallback()
            this.closeForm()
        }
    }

    closeForm() {
        this.style.display = "none"
    }

    OnResize(){
        if(window.getComputedStyle(this.parentElement).display == "none")
            return;

        let containerWidth = this.previewSceneContainer.clientWidth
        let containerHeight = this.previewSceneContainer.clientHeight

        if(containerWidth <= 0 || containerHeight <= 0)
            return

        let margin = 0
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
        this.RedrawFrame()
    }

    RedrawFrame(){
        let prevStore = huahuoEngine.GetCurrentStoreId()

        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let mainStoreId = mainSceneView.storeId
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainStoreId) // Only render the main store.i.e. the 1st store.

        let currentLayer = huahuoEngine.GetCurrentLayer()
        let currentFrameId = currentLayer.GetCurrentFrame()
        let previousCanvas = renderEngine2D.setDefaultCanvas(this.previewCanvas)

        this.previewAnimationPlayer.storeId = mainStoreId
        this.previewAnimationPlayer.loadShapesFromStore()
        this.previewAnimationPlayer.setFrameId(currentFrameId)
        if(previousCanvas)
            renderEngine2D.setDefaultCanvas(previousCanvas)

        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStore)
    }
}

export {ProjectInfoForm}