import {CustomElement} from "hhcommoncomponents";

import {SVGFiles} from "../Utilities/Svgs";
import {binaryFileUploader} from "../RESTApis/BinaryFileUploader";
import huahuoProperties from "/dist/hhide.properties";
import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {api} from "../RESTApis/RestApi"
import {ProjectListForm} from "./ProjectListForm";
import {formManager} from "../Utilities/FormManager";
import {ProjectInfoForm} from "./ProjectInfoForm";
import {projectInfo} from "../SceneView/ProjectInfo";
import {projectManager} from "../HuaHuoEngine/ProjectManager";
import {ExportImageForm} from "./ExportForm";
import {fileLoader} from "../SceneView/FileLoader";
import {ProjectListFormX} from "./ProjectListFormX";

@CustomElement({
    selector: "hh-tool-bar"
})
class HHToolBar extends HTMLElement {
    saveButton: HTMLButtonElement
    loadButton: HTMLButtonElement
    previewButton: HTMLButtonElement
    projectListButton: HTMLButtonElement
    elementListButton: HTMLButtonElement
    uploadButton: HTMLButtonElement

    exportImageButton: HTMLButtonElement

    createButton(svgFile, title, onClick) {
        let btn = document.createElement("button")
        btn.className = "btn btn-outline-secondary"
        btn.style.width = "40px"
        btn.style.height = "40px"
        btn.innerHTML = svgFile
        btn.title = title
        btn.addEventListener("click", onClick)

        this.appendChild(btn)
    }

    constructor() {
        super();


        this.className = "btn-group btn-group-sm"

        i18n.ExecuteAfterInited(function () {
            this.saveButton = this.createButton(SVGFiles.saveBtn, i18n.t("hint.saveLocal"), this.save.bind(this))

            this.loadButton = this.createButton(SVGFiles.loadBtn, i18n.t("hint.loadLocal"), this.onFileSelected.bind(this))

            this.previewButton = this.createButton(SVGFiles.previewBtn, i18n.t("hint.preview"), this.uploadAndOpenPlayer.bind(this))

            this.elementListButton = this.createButton(SVGFiles.elementListButton, i18n.t("hint.library"), () => {
                this.listElements()
            })

            this.projectListButton = this.createButton(SVGFiles.projectListBtn, i18n.t("hint.listProject"), () => {
                this.listProjects()
            })

            this.uploadButton = this.createButton(SVGFiles.uploadBtn, i18n.t("hint.uploadProject"), () => {
                this.uploadProject()
            })

            this.exportImageButton = this.createButton(SVGFiles.exportImage, i18n.t("hint.exportImage"), () => {
                this.exportImage()
            })
        }.bind(this))
    }

    @NeedLogin()
    uploadProject(afterAction: Function = null) {
        if (!projectInfo.inited) {
            // Prompt the Project description page.
            let storeInforForm = formManager.openForm(ProjectInfoForm)
            storeInforForm.onOKCallback = () => {
                binaryFileUploader.upload().then((response) => {
                    if (afterAction)
                        afterAction(response.data)
                })
            }
        } else {
            binaryFileUploader.upload().then((response) => {
                if (afterAction)
                    afterAction(response.data)
            })
        }
    }

    uploadAndOpenPlayer() {
        this.uploadProject((response) => {
            let fileId = response["fileId"]

            let playerUrl = huahuoProperties["huahuo.player.url"] + "?projectId=" + fileId

            window.open(playerUrl, '_blank')
        })
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
                let file = (<HTMLInputElement>evt.target).files[0];
                fileLoader.loadImageFile(file)
            }
        })
    }

    save() {
        projectManager.save()
    }

    @NeedLogin()
    listProjects(pageSize: number = 12) {
        formManager.openReactForm(ProjectListFormX, {
            title: i18n.t("yourProjects"),
            pageSize: pageSize,
            listUpdateFunction: api.listProjects.bind(api)
        })
    }

    deleteElement(elementId) {

    }

    @NeedLogin()
    listElements(pageSize: number = 10) {
        formManager.openReactForm(ProjectListFormX, {
            title: i18n.t("allElements"),
            pageSize: pageSize,
            listUpdateFunction: api.listElements.bind(api)
        })
    }

    exportImage() {
        formManager.openForm(ExportImageForm)
    }
}

export {HHToolBar}