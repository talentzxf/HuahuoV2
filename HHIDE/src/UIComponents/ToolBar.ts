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

function uploadAndOpenPlayer(){
    dataFileUploader.upload().then((response)=>{
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
    constructor() {
        super();

        this.saveButton = document.createElement("button")
        this.saveButton.style.width = "30px"
        this.saveButton.style.height = "30px"
        this.saveButton.innerHTML = SVGFiles.saveBtn

        this.saveButton.addEventListener("click", save)
        this.appendChild(this.saveButton)

        this.loadButton = document.createElement("button")
        this.loadButton.style.width = "30px"
        this.loadButton.style.height = "30px"
        this.loadButton.innerHTML = SVGFiles.loadBtn

        this.loadButton.addEventListener("click", this.onFileSelected.bind(this))
        this.appendChild(this.loadButton)

        this.previewButton = document.createElement("button")
        this.previewButton.style.width = "30px"
        this.previewButton.style.height = "30px"
        this.previewButton.innerHTML = SVGFiles.previewBtn
        this.previewButton.addEventListener("click", uploadAndOpenPlayer)
        this.appendChild(this.previewButton)
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
}

export {HHToolBar}