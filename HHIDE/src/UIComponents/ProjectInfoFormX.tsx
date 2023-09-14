import * as React from "react"
import {CloseBtn} from "./CloseBtn";
import {CSSUtils} from "../Utilities/CSSUtils";
import {SVGFiles} from "../Utilities/Svgs";
import {api} from "../RESTApis/RestApi";
import {huahuoEngine, Player, renderEngine2D} from "hhenginejs";
import {SceneView} from "../SceneView/SceneView";
import {SnapshotUtils} from "../Utilities/SnapshotUtils";
import {projectInfo} from "../SceneView/ProjectInfo";
import {HHToast} from "hhcommoncomponents";
import {FormProps} from "../Utilities/FormManager";

function validateText(val: string): boolean {
    return /^[a-zA-Z0-9_-]+$/.test(val)
}

function getButtonClass(color: string) {
    let btnClass: string = `text-white bg-${color}-600 hover:bg-${color}-700 focus:ring-4 focus:outline-none focus:ring-${color}-300 ` +
        `font-medium text-center dark:bg-${color}-600 dark:hover:bg-${color}-700 dark:focus:ring-${color}-800 px-10 mt-5`
    return btnClass
}

function RenderPreviewCanvas(storeId, player, canvas, frameId) {
    let prevPlayer = huahuoEngine.getActivePlayer()
    let prevStore = huahuoEngine.GetCurrentStoreId()

    let previousCanvas = null

    try {
        huahuoEngine.setActivePlayer(player)
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(storeId) // Only render the main store.i.e. the 1st store.
        previousCanvas = renderEngine2D.setDefaultCanvas(canvas)

        player.storeId = storeId
        player.loadShapesFromStore()
        player.setFrameId(frameId)
    } finally {
        if (previousCanvas)
            renderEngine2D.setDefaultCanvas(previousCanvas)
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStore)
        if (prevPlayer) {
            huahuoEngine.setActivePlayer(prevPlayer)
        }
    }
}

type ProjectInfoState = {
    projectName: string
    projectDescription: string
    isNameValid: boolean
    canvasWidth: number
    canvasHeight: number
    canvasDisplayWidth: number
    canvasDisplayHeight: number
}

type ProjectInfoFormProps = FormProps & {
    onOKCallback?: Function
}

class ProjectInfoFormX extends React.Component<ProjectInfoFormProps, ProjectInfoState> {
    state: ProjectInfoState = {
        projectName: "",
        projectDescription: "",
        isNameValid: false,
        canvasWidth: 300,
        canvasHeight: 300,
        canvasDisplayWidth: 300,
        canvasDisplayHeight: 300
    }

    previewCanvasRef
    previewAnimationPlayer = new Player()

    constructor(props) {
        super(props);

        this.previewCanvasRef = React.createRef()
    }

    onProjectNameChange(e) {
        let projectNameCandidate = e.currentTarget.value
        this.state.projectName = projectNameCandidate
        if (validateText(projectNameCandidate)) {
            api.checkFileNameExistence(projectNameCandidate).then((result) => {
                if (result && result.data && result.data["exist"] == false) { // The project name doesn't exist in the system now. Name is valid
                    this.state.isNameValid = true
                } else {
                    this.state.isNameValid = false
                }

                this.setState(this.state)
            })
        } else {
            this.state.isNameValid = false
            this.setState(this.state)
        }
    }

    onProjectDescriptionChange(e) {
        this.state.projectDescription = e.currentTarget.value
        this.setState(this.state)
    }

    componentDidMount() {
        let prevCanvas = renderEngine2D.getDefaultCanvas()
        renderEngine2D.init(this.previewCanvasRef.current)

        if (prevCanvas) // Restore the previous canvas as default. Or else functionality will be broken.
            renderEngine2D.setDefaultCanvas(prevCanvas)

        let [initW, initH] = renderEngine2D.getInitCanvasWH()

        if (initW > 0) {

            let [resultW, resultH] = renderEngine2D.getContentWH(initW, initH)

            this.state.canvasWidth = initW
            this.state.canvasHeight = initH * initW / resultW
            this.state.canvasDisplayWidth = Math.floor(this.state.canvasWidth / 5.0)
            this.state.canvasDisplayHeight = Math.floor(this.state.canvasHeight / 5.0)

            renderEngine2D.resize(this.previewCanvasRef.current, initW, initH)
        }
        this.RedrawFrame()

        this.setState(this.state)
    }

    RedrawFrame() {
        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let mainStoreId = mainSceneView.storeId
        let currentLayer = huahuoEngine.GetCurrentLayer()
        let currentFrameId = currentLayer.GetCurrentFrame()

        RenderPreviewCanvas(mainStoreId, this.previewAnimationPlayer, this.previewCanvasRef.current, currentFrameId)
    }

    onOK(e) {
        e.preventDefault()
        e.stopPropagation()

        let projectName = this.state.projectName
        if (!validateText(projectName)) {
            HHToast.warn("Invalid Project Name:" + projectName)
            return;
        }

        let coverPageBinary = SnapshotUtils.takeSnapshot(this.previewCanvasRef.current)
        projectInfo.Setup(projectName, this.state.projectDescription, coverPageBinary)

        if (this.props.onOKCallback) {
            this.props.onOKCallback()
        }

        this.props?.closeForm()
    }

    render() {
        return (
            <div className="select-none flex flex-col items-center justify-center mx-auto">
                <div
                    className="bg-white rounded-lg drop-shadow-2xl dark:border md:mt-0 sm:max-w-md xl:p-0
                        dark:bg-gray-800 dark:border-gray-700">
                    <form className="p-4 space-y-4 divide-y divide-gray-300" action="#">
                        <div className="flex align-middle">
                            <h5 className="px-2 text-xl font-bold leading-tight tracking-tight text-gray-900 md:text-2xl dark:text-white">
                                {i18n.t("ProjectInfo")}
                            </h5>
                            <CloseBtn onclick={this.props?.closeForm}></CloseBtn>
                        </div>
                        <div className="w-full flex flex-col">
                            <label className="font-bold">{i18n.t("ProjectName")}</label>
                            <div className="flex items-center">
                                <input className={CSSUtils.getInputStyle() + " w-full"}
                                       placeholder="Enter Project Name" onChange={this.onProjectNameChange.bind(this)}
                                       value={this.state.projectName}/>
                                <img className="w-[20px] h-[20px]"
                                     src={this.state.isNameValid ? SVGFiles.okImg : SVGFiles.notOKImg}/>
                            </div>
                            <label className="font-bold">{i18n.t("ProjectDescription")}</label>
                            <textarea placeholder="Enter Project Description"
                                      className="block p-2.5 w-full text-sm text-gray-900 bg-gray-50 rounded-lg border border-gray-300 focus:ring-blue-500 focus:border-blue-500 dark:bg-gray-700 dark:border-gray-600 dark:placeholder-gray-400 dark:text-white dark:focus:ring-blue-500 dark:focus:border-blue-500"
                                      value={this.state.projectDescription}
                                      onChange={this.onProjectDescriptionChange.bind(this)}></textarea>
                            <label className="font-bold">{i18n.t("ProjectPreview")}</label>
                            <div>
                                <canvas ref={this.previewCanvasRef}
                                        className={`border-cyan-200 border-2`}
                                        style={{
                                            width: `${this.state.canvasDisplayWidth}px`,
                                            height: `${this.state.canvasDisplayHeight}px`
                                        }}
                                        width={this.state.canvasWidth}
                                        height={this.state.canvasHeight}
                                ></canvas>
                            </div>
                        </div>
                        <div className="w-full flex flex-row">
                            <div className="w-full"></div>
                            <button className={getButtonClass("primary") + " text-lg rounded"}
                                    onClick={this.onOK.bind(this)}>OK
                            </button>
                            <div className="w-full"></div>
                        </div>
                    </form>
                </div>
            </div>
        )
    }
}

export {ProjectInfoFormX, RenderPreviewCanvas}