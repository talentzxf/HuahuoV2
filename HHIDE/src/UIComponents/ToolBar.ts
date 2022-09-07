import {CustomElement} from "hhcommoncomponents";
import {huahuoEngine} from "hhenginejs";
import {Logger} from "hhcommoncomponents";
import {SceneView} from "../SceneView/SceneView";
import {HHTimeline} from "hhtimeline"
import {saveAs} from 'file-saver';

declare var Module:any

function save() {
    // Restore current scene view.

    let mainSceneView:SceneView = document.querySelector("#mainScene")
    let oldStoreId = huahuoEngine.GetCurrentStoreId()

    try{
        console.log("Setting default store by index:" + mainSceneView.storeId)
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainSceneView.storeId)
        let Uint8Array = Module.writeObjectStoreInMemoryFile()
        let blob = new Blob([Uint8Array], {type: "application/octet-stream"})
        saveAs(blob, "huahuo.data")
        Logger.info("Good!!")
    }finally {
        console.log("Setting default store by index asdfasdfasdf:" + oldStoreId)
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
        } else {
            console.log("Can't load file: " + storeMemoryFile)
        }
    }
    reader.readAsArrayBuffer(file)
}

@CustomElement({
    selector: "hh-tool-bar"
})
class HHToolBar extends HTMLElement{
    saveButton: HTMLButtonElement
    loadButton: HTMLButtonElement
    constructor() {
        super();

        //                <button onclick="window.menuoperations.save()">save</button>
        //                 <input type="file" onchange="window.menuoperations.load(this.value, event)">
        //                 <button onclick="window.menuoperations.uploadAndOpenPlayer()">uploadAndOpen</button>

        this.saveButton = document.createElement("button")
        this.saveButton.innerText = "Save"

        this.saveButton.addEventListener("click", save)
        this.appendChild(this.saveButton)

        this.loadButton = document.createElement("button")
        this.loadButton.innerText = "Load"
        this.loadButton.addEventListener("click", this.onFileSelected.bind(this))
        this.appendChild(this.loadButton)
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