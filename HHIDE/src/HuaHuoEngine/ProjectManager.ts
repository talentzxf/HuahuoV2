import {Logger, HHToast, getFileNameFromGZip} from "hhcommoncomponents";
import {huahuoEngine} from "hhenginejs";
import {HHTimeline} from "hhtimeline"
import {api} from "../RESTApis/RestApi";
import {gzipSync, gunzipSync} from "fflate"
import {SceneView} from "../SceneView/SceneView";
import {saveAs} from 'file-saver';
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";

declare var Module: any

class ProjectManager {
    load(fName: string, e) {
        Logger.info("Opening:" + fName)

        let _this = this
        let file = e.target.files[0];
        let reader = new FileReader()
        reader.onload = function (e: ProgressEvent<FileReader>) {
            let arrayBuffer = e.target.result

            _this.loadFromArrayBuffer(arrayBuffer)

            IDEEventBus.getInstance().emit(EventNames.FILELOADED, fName)
        }
        reader.readAsArrayBuffer(file)
    }

    loadFromServer(projectId: number) {
        let _this = this
        return api.downloadProject(projectId).then(function (response: ArrayBuffer) {
            _this.loadFromArrayBuffer(response["data"])
        })
    }

    loadFromArrayBuffer(arrayBuffer) {
        let compressedFileContent = new Uint8Array(arrayBuffer as ArrayBuffer);
        let fileContent = gunzipSync(compressedFileContent)
        let fileName = getFileNameFromGZip(compressedFileContent)

        let storeMemoryFile = "mem://" + fileName;
        let fileSize = fileContent.length;
        let memoryFileContent = Module.createMemFile(storeMemoryFile, fileSize);
        for (let i = 0; i < fileSize; i++) { // Copy to the file, byte by byte
            memoryFileContent[i] = fileContent[i];
        }

        Module.setStoreFilePath(storeMemoryFile)
        let result = Module.LoadStoreFileCompletely(storeMemoryFile);
        if (result == 0) { // TODO: Should send out event
            let timeline: HHTimeline = document.querySelector("hh-timeline")
            timeline.reloadTracks();
            HHToast.info(i18n.t("toast.openProjectSucceeded"))
        } else {
            HHToast.error(i18n.t("toast.openProjectFailed"))
        }

        // Set scene view storeId as the newly loaded storeId.
        let mainSceneView: SceneView = document.querySelector("#mainScene")
        mainSceneView.setStoreId(huahuoEngine.GetCurrentStoreId())
        huahuoEngine.getActivePlayer().updateAllShapes(true)
    }

    getProjectData(){
        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let oldStoreId = huahuoEngine.GetCurrentStoreId()

        try{
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainSceneView.storeId)
            let Uint8Array = Module.writeAllObjectsInMemoryFile()

            let storeFilePathArray = Module.getStoreFilePath().split(/[/\\]/)
            let storeFileName = storeFilePathArray[storeFilePathArray.length - 1]
            let CompressedFileContent = gzipSync(Uint8Array, {filename: storeFileName});
            return new Blob([CompressedFileContent], {type: "application/octet-stream"})
        } finally {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(oldStoreId)
        }
    }

    save() {
        // Restore current scene view.
        try {
            saveAs(this.getProjectData(), "huahuo_project.hua")
            HHToast.info(i18n.t("toast.projectSaved"))
        } catch (e) {
            HHToast.info(i18n.t("toast.projectSaveFailed"))
        }
    }
}

let projectManager = new ProjectManager()
export {projectManager}