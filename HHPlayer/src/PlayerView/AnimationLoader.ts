import {fileDownloader} from "../RestApis/FileDownloader";
import {Logger, getFileNameFromGZip} from "hhcommoncomponents";
import {PlayerView} from "./PlayerView";
import {huahuoEngine} from "hhenginejs"
import {gunzipSync} from "fflate"
import {eventBus} from "hhcommoncomponents";

declare var Module: any;

enum AnimationLoaderEvents{
    DOWNLOADBEGIN = "animationDownloadBegin",
    DOWNLOADED = "animationDownloaded",
    DOWNLOADFAILED = "animationDownloadFailed",
    LOADBEGIN = "animationLoadBegin",
    LOADED = "animationLoaded",
    LOADFAILED = "animationLoadFailed"
}

class AnimationLoader{
    constructor() {
        for(let evtName of Object.values(AnimationLoaderEvents)){
            eventBus.registerEvent(evtName, "HHPlayer")
        }
    }

    projectId:string = "unknownProject"
    loadAnimation(projectId: string){
        this.projectId = projectId

        // 1. Wait for the PlayerView ready.

        let playerView:PlayerView = document.querySelector("hh-player")

        let _this = this
        playerView.executeAfterInit(function(){
            eventBus.triggerEvent(AnimationLoaderEvents.DOWNLOADBEGIN)
            // 2. Download the file
            fileDownloader.downloadFile(projectId, _this.onProjectDownloaded.bind(_this), _this.onFailed.bind(_this))
        })
    }

    onProjectDownloaded(data: Blob, fileName: string){
        eventBus.triggerEvent(AnimationLoaderEvents.DOWNLOADED)
        Logger.info("Project:" + this.projectId + " downloaded!")

        eventBus.triggerEvent(AnimationLoaderEvents.LOADBEGIN)
        Promise.resolve( data.arrayBuffer() ).then(
            (arrayBuffer)=>{
                let compressedFileContent = new Uint8Array(arrayBuffer as ArrayBuffer);
                let fileContent = gunzipSync(compressedFileContent)
                let fileName = getFileNameFromGZip(compressedFileContent)

                let storeMemoryFile = "mem://" + fileName;
                let fileSize = fileContent.length;

                huahuoEngine.ExecuteAfterInited(() => {
                    let memoryFileContent = Module.createMemFile(storeMemoryFile, fileSize);
                    for (let i = 0; i < fileSize; i++) { // Copy to the file, byte by byte
                        memoryFileContent[i] = fileContent[i];
                    }

                    Module.setStoreFilePath(storeMemoryFile)

                    let result = Module.LoadStoreFileCompletely(storeMemoryFile);
                    if (result == 0){
                        eventBus.triggerEvent(AnimationLoaderEvents.LOADED)
                        Logger.info("File successfully loaded:" + storeMemoryFile)
                    }
                    else{
                        eventBus.triggerEvent(AnimationLoaderEvents.LOADFAILED)
                        Logger.error("Can't load file: " + storeMemoryFile)
                    }
                })
            }
        ).catch((error)=>{
            Logger.error("Error happened:" + error)
        })
    }

    onFailed(status:string, msg: string){
        Logger.error("Can't download project:" + this.projectId + " Status:" + status + " Msg:" + msg)
        eventBus.triggerEvent(AnimationLoaderEvents.DOWNLOADFAILED)
    }
}

let animationLoader = new AnimationLoader()

export {animationLoader, AnimationLoaderEvents}