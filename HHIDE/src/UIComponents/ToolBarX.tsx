import * as React from "react"
import {SVGFiles} from "../Utilities/Svgs";
import {projectManager} from "../HuaHuoEngine/ProjectManager";
import {fileLoader} from "../SceneView/FileLoader";
import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {formManager} from "../Utilities/FormManager";
import {ProjectListFormX} from "./ProjectListFormX";
import {api} from "../RESTApis/RestApi";
import huahuoProperties from "/dist/hhide.properties";
import {projectInfo} from "../SceneView/ProjectInfo";
import {ProjectInfoFormX} from "./ProjectInfoFormX";
import {binaryFileUploader} from "../RESTApis/BinaryFileUploader";
import {ExportImageForm} from "./ExportForm";
import {undoManager} from "../RedoUndo/UndoManager";
import {imgButton} from "./MainMenuX";

class ToolBarX extends React.Component<any, any> {
    save() {
        projectManager.save()
    }

    onFileSelected() {
        let hiddenFileButton = document.createElement("input")
        hiddenFileButton.type = "file"
        hiddenFileButton.click()

        hiddenFileButton.addEventListener("change", (evt) => {
            let fName = hiddenFileButton.value

            if (fName.endsWith(".hua")) {
                projectManager.load(fName, evt)
            } else if (fName.match(/\.(jpg|jpeg|png|gif|svg|webp)$/i)) { // Is Image, load image.
                let file = (evt.target as HTMLInputElement).files[0];
                fileLoader.loadImageFile(file)
            }
        })
    }

    @NeedLogin()
    listProjects(pageSize: number = 12) {
        formManager.openReactForm(ProjectListFormX, {
            title: i18n.t("yourProjects"),
            pageSize: pageSize,
            listUpdateFunction: api.listProjects.bind(api),
            writeAuthInfo: false
        })
    }

    @NeedLogin()
    listElements(pageSize: number = 12) {
        formManager.openReactForm(ProjectListFormX, {
            title: i18n.t("allElements"),
            pageSize: pageSize,
            listUpdateFunction: api.listElements.bind(api),
            writeAuthInfo: true
        })
    }

    uploadAndOpenPlayer() {
        this.uploadProject((response) => {
            let fileId = response["fileId"]

            let playerUrl = huahuoProperties["huahuo.player.url"] + "?projectId=" + fileId

            window.open(playerUrl, '_blank')
        })
    }

    @NeedLogin()
    uploadProject(afterAction: Function = null) {
        if (!projectInfo.inited) {
            formManager.openReactForm(ProjectInfoFormX, {
                onOKCallback: () => {
                    binaryFileUploader.upload().then((response) => {
                        if (afterAction)
                            afterAction(response.data)
                    })
                }
            })

        } else {
            binaryFileUploader.upload().then((response) => {
                if (afterAction)
                    afterAction(response.data)
            })
        }
    }

    exportImage() {
        formManager.openForm(ExportImageForm)
    }

    undo() {
        undoManager.UnDo()
    }

    redo() {
        undoManager.ReDo()
    }

    render() {
        return (
            <div className="flex flex-row">
                {imgButton(SVGFiles.saveBtn, i18n.t("hint.saveLocal"), this.save.bind(this))}
                {imgButton(SVGFiles.loadBtn, i18n.t("hint.loadLocal"), this.onFileSelected.bind(this))}
                {imgButton(SVGFiles.previewBtn, i18n.t("hint.preview"), this.uploadAndOpenPlayer.bind(this))}
                {imgButton(SVGFiles.elementListButton, i18n.t("hint.library"), () => {
                    this.listElements()
                })}
                {imgButton(SVGFiles.projectListBtn, i18n.t("hint.listProject"), () => {
                    this.listProjects()
                })}
                {imgButton(SVGFiles.uploadBtn, i18n.t("hint.uploadProject"), () => {
                    this.uploadProject()
                })}
                {imgButton(SVGFiles.exportImage, i18n.t("hint.exportImage"), () => {
                    this.exportImage()
                })}
            </div>
        )
    }
}

export {ToolBarX}