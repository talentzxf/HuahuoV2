import {HHContainer} from "hhpanel";
import {HHPanel} from 'hhpanel'
import {HHContent} from 'hhpanel'
import {HHTitle} from "hhpanel";
import {HHTimeline} from "hhtimeline";
import {NavTree} from "./UIComponents/NavTree"
import {DrawToolBar} from "./UIComponents/DrawToolBar";
import {SceneView} from "./SceneView/SceneView";
import {Inspector} from "./Inspector/Inspector";
import {LoginForm} from "./Identity/LoginForm";
import {RegisterForm} from "./Identity/RegisterForm";
import {UserInfoBar} from "./Identity/UserInfoBar";
import {dataFileUploader} from "./RESTApis/DataFileUploader";
import i18next from "i18next"

import "vanilla-colorful"

import {library, dom} from "@fortawesome/fontawesome-svg-core";
import {faPlus} from "@fortawesome/free-solid-svg-icons/faPlus";
import {faMinus} from "@fortawesome/free-solid-svg-icons/faMinus"
import {faSlash} from "@fortawesome/free-solid-svg-icons/faSlash";
import {faCircle} from "@fortawesome/free-solid-svg-icons/faCircle";
import {faCircleXmark} from "@fortawesome/free-regular-svg-icons";
import {faBezierCurve} from "@fortawesome/free-solid-svg-icons/faBezierCurve";
import {faArrowPointer} from "@fortawesome/free-solid-svg-icons";
import {faSquare} from "@fortawesome/free-solid-svg-icons";
import {faFileAudio} from "@fortawesome/free-regular-svg-icons";
import {faEye} from "@fortawesome/free-regular-svg-icons"
import {faEyeSlash} from "@fortawesome/free-regular-svg-icons";
import {faEdit} from "@fortawesome/free-regular-svg-icons";
import {faFileImage} from "@fortawesome/free-regular-svg-icons";
import {faTimesCircle} from "@fortawesome/free-regular-svg-icons";
import {saveAs} from 'file-saver';
import {Logger} from "hhcommoncomponents";
import {huahuoEngine} from "hhenginejs";
import huahuoProperties from "./hhide.properties";
import {init} from "./init"

import "./i18nInit"

library.add(faMinus)
library.add(faPlus)
library.add(faSlash)
library.add(faCircle)
library.add(faBezierCurve)
library.add(faArrowPointer)
library.add(faSquare)
library.add(faFileAudio)
library.add(faEye)
library.add(faEyeSlash)
library.add(faEdit)
library.add(faFileImage)
library.add(faTimesCircle)
library.add(faCircleXmark)
dom.watch();

function save() {
    // Restore current scene view.

    let mainSceneView = document.querySelector("#mainScene")
    let oldStoreId = huahuoEngine.GetCurrentStoreId()

    try{
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainSceneView.storeId)
        let Uint8Array = Module.writeObjectStoreInMemoryFile()
        let blob = new Blob([Uint8Array], {type: "application/octet-stream"})
        saveAs(blob, "huahuo.data")
        Logger.info("Good!!")
    }finally {
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(oldStoreId)
    }
}

function load(fName, e) {
    Logger.info("Opening:" + fName)

    let fileName = fName.split("\\").pop();
    let file = e.target.files[0];
    let reader = new FileReader()
    reader.onload = function (e) {
        let fileContent = new Uint8Array(e.target.result);
        let storeMemoryFile = "mem://" + fileName;
        let fileSize = fileContent.length;
        let memoryFileContent = Module.createMemFile(storeMemoryFile, fileSize);
        for (let i = 0; i < fileSize; i++) { // Copy to the file, byte by byte
            memoryFileContent[i] = fileContent[i];
        }

        let result = Module.LoadStoreFileCompletely(storeMemoryFile);
        if (result == 0) {
            let timeline = document.querySelector("hh-timeline")
            timeline.reloadTracks();
        } else {
            console.log("Can't load file: " + storeMemoryFile)
        }
    }
    reader.readAsArrayBuffer(file)
}

function uploadAndOpenPlayer(){
    dataFileUploader.upload().then((response)=>{
        let fileId = response.fileId

        let playerUrl = huahuoProperties["huahuo.player.url"] + "?projectId=" + fileId

        window.open(playerUrl, '_blank')
    })
}

window.menuoperations = {
    save: save,
    load: load,
    uploadAndOpenPlayer: uploadAndOpenPlayer
}

init()