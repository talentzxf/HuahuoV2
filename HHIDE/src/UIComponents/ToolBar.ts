import {CustomElement} from "hhcommoncomponents";
import {huahuoEngine} from "hhenginejs";
import {Logger} from "hhcommoncomponents";
import {SceneView} from "../SceneView/SceneView";
import {HHTimeline} from "hhtimeline"
import {saveAs} from 'file-saver';
import {SVGFiles} from "../Utilities/Svgs";
import {dataFileUploader} from "../RESTApis/DataFileUploader";
import huahuoProperties from "../hhide.properties";
import {HHToast} from "hhcommoncomponents";
import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {api} from "../RESTApis/RestApi"
import {ProjectListForm} from "../Utilities/ProjectListForm";
import {formManager} from "../Utilities/FormManager";
import {StoreInfoForm} from "../Utilities/StoreInfoForm";

declare var Module:any

function save() {
    // Restore current scene view.

    let mainSceneView:SceneView = document.querySelector("#mainScene")
    let oldStoreId = huahuoEngine.GetCurrentStoreId()

    try{
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainSceneView.storeId)
        let Uint8Array = Module.writeObjectStoreInMemoryFile()
        let blob = new Blob([Uint8Array], {type: "application/octet-stream"})
        saveAs(blob, "huahuo.data")
        HHToast.info(i18n.t("toast.projectSaved"))
    }
    catch (e){
        HHToast.info(i18n.t("toast.projectSaveFailed"))
    }
    finally {
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(oldStoreId)
    }
}

function load(fName:string, e) {
    Logger.info("Opening:" + fName)

    let fileName = fName.split("\\").pop();
    let file = e.target.files[0];
    let reader = new FileReader()
    reader.onload = function (e:ProgressEvent<FileReader>) {
        let arrayBuffer = e.target.result
        let fileContent = new Uint8Array(arrayBuffer as ArrayBuffer);
        let storeMemoryFile = "mem://" + fileName;
        let fileSize = fileContent.length;
        let memoryFileContent = Module.createMemFile(storeMemoryFile, fileSize);
        for (let i = 0; i < fileSize; i++) { // Copy to the file, byte by byte
            memoryFileContent[i] = fileContent[i];
        }

        let result = Module.LoadStoreFileCompletely(storeMemoryFile);
        if (result == 0) {
            let timeline:HHTimeline = document.querySelector("hh-timeline")
            timeline.reloadTracks();
            HHToast.info(i18n.t("toast.openProjectSucceeded"))
        } else {
            HHToast.error(i18n.t("toast.openProjectFailed"))
        }
    }
    reader.readAsArrayBuffer(file)
}

function uploadProject(afterAction:Function = null){
    // Prompt the Project description page.
    let storeInforForm = formManager.openForm(StoreInfoForm)

    storeInforForm.onOKCallback = ()=>{
        dataFileUploader.upload().then((response)=>{
            if(afterAction)
                afterAction(response)
        })
    }
}

function uploadAndOpenPlayer(){
    uploadProject((response)=>{
        let fileId = response["fileId"]

        let playerUrl = huahuoProperties["huahuo.player.url"] + "?projectId=" + fileId

        window.open(playerUrl, '_blank')
    })
}



@CustomElement({
    selector: "hh-tool-bar"
})
class HHToolBar extends HTMLElement{
    saveButton: HTMLButtonElement
    loadButton: HTMLButtonElement
    previewButton: HTMLButtonElement
    projectListButton: HTMLButtonElement
    uploadButton: HTMLButtonElement
    constructor() {
        super();

        i18n.ExecuteAfterInited(function(){
            this.saveButton = document.createElement("button")
            this.saveButton.style.width = "30px"
            this.saveButton.style.height = "30px"
            this.saveButton.innerHTML = SVGFiles.saveBtn
            this.saveButton.title = i18n.t("hint.saveLocal")
            this.saveButton.addEventListener("click", save)
            this.appendChild(this.saveButton)

            this.loadButton = document.createElement("button")
            this.loadButton.style.width = "30px"
            this.loadButton.style.height = "30px"
            this.loadButton.title = i18n.t("hint.localLocal")
            this.loadButton.innerHTML = SVGFiles.loadBtn

            this.loadButton.addEventListener("click", this.onFileSelected.bind(this))
            this.appendChild(this.loadButton)

            this.previewButton = document.createElement("button")
            this.previewButton.style.width = "30px"
            this.previewButton.style.height = "30px"
            this.previewButton.innerHTML = SVGFiles.previewBtn
            this.previewButton.title = i18n.t("hint.preview")
            this.previewButton.addEventListener("click", uploadAndOpenPlayer)
            this.appendChild(this.previewButton)

            this.projectListButton = document.createElement("button")
            this.projectListButton.style.width = "30px"
            this.projectListButton.style.height = "30px"
            this.projectListButton.innerHTML = SVGFiles.projectListBtn
            this.projectListButton.title = i18n.t("hint.listProject")
            this.projectListButton.addEventListener("click", this.listProjects.bind(this))
            this.appendChild(this.projectListButton)

            this.uploadButton = document.createElement("button")
            this.uploadButton.style.width = "30px"
            this.uploadButton.style.height = "30px"
            this.uploadButton.innerHTML = SVGFiles.uploadBtn
            this.uploadButton.title = i18n.t("hint.uploadProject")
            this.uploadButton.onclick = function(){
                uploadProject()
            }
            this.appendChild(this.uploadButton)

        }.bind(this))
    }

    onFileSelected(){
        let hiddenFileButton = document.createElement("input")
        hiddenFileButton.type = "file"
        hiddenFileButton.click()

        hiddenFileButton.addEventListener("change", (evt)=>{
            let fName = hiddenFileButton.value
            load(fName, evt)
        })
    }

    @NeedLogin()
    listProjects(pageNo:number = 0, pageSize:number = 10){
        api.listProjects((listProjectResult)=>{
            let form = formManager.openForm(ProjectListForm)
            let totalPage = listProjectResult.totalCount/pageSize

            form.updateProjectList(totalPage, pageNo, listProjectResult.projectFiles)
        })
    }
}

export {HHToolBar}