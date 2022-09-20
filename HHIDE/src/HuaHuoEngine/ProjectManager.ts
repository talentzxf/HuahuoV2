import {Logger, HHToast} from "hhcommoncomponents";
import {huahuoEngine} from "hhenginejs";
import {HHTimeline} from "hhtimeline"
import {api} from "../RESTApis/RestApi";
import {gzipSync, gunzipSync} from "fflate"
import {SceneView} from "../SceneView/SceneView";
import {saveAs} from 'file-saver';

declare var Module: any

function getFileNameFromGZip(d: Uint8Array){
    if (d[0] != 31 || d[1] != 139 || d[2] != 8){
        HHToast.error("Wrong file format")
        return
    }

    let flg = d[3];
    let st = 10;
    if (flg & 4)
        st += d[10] | (d[11] << 8) + 2;

    let fileName = ""
    // TODO: Is this the right way to get the filename??
    for (let i = st; d[st]>0; st++){
        fileName += String.fromCharCode(d[st])
    }

    return fileName
}

class ProjectManager {
    load(fName: string, e) {
        Logger.info("Opening:" + fName)

        let _this = this
        let file = e.target.files[0];
        let reader = new FileReader()
        reader.onload = function (e: ProgressEvent<FileReader>) {
            let arrayBuffer = e.target.result

            _this.loadFromArrayBuffer(arrayBuffer)
        }
        reader.readAsArrayBuffer(file)
    }

    loadFromServer(projectId: number) {
        let _this = this
        api.downloadProject(projectId).then(function (blob: Blob) {
            Promise.resolve(blob.arrayBuffer()).then((data) => {
                _this.loadFromArrayBuffer(data)
            })
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
    }

    save() {
        // Restore current scene view.

        let mainSceneView: SceneView = document.querySelector("#mainScene")
        let oldStoreId = huahuoEngine.GetCurrentStoreId()

        try {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainSceneView.storeId)
            let Uint8Array = Module.writeObjectStoreInMemoryFile()

            let storeFilePathArray = Module.getStoreFilePath().split(/[/\\]/)
            let storeFileName = storeFilePathArray[storeFilePathArray.length - 1]
            let CompressedFileContent = gzipSync(Uint8Array, {filename: storeFileName});
            let blob = new Blob([CompressedFileContent], {type: "application/octet-stream"})
            saveAs(blob, "huahuo_project.hua")
            HHToast.info(i18n.t("toast.projectSaved"))
        } catch (e) {
            HHToast.info(i18n.t("toast.projectSaveFailed"))
        } finally {
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(oldStoreId)
        }
    }
}

let projectManager = new ProjectManager()
export {projectManager}