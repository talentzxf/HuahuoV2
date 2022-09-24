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
    }

    triggerEvent(evtName:string){
        eventBus.triggerEvent("HHPlayer", evtName)
    }

    projectId:string = "unknownProject"
    loadAnimation(projectId: string){
        this.projectId = projectId

        // 1. Wait for the PlayerView ready.

        let playerView:PlayerView = document.querySelector("hh-player")

        let _this = this
        playerView.executeAfterInit(function(){
            _this.triggerEvent(AnimationLoaderEvents.DOWNLOADBEGIN)
            // 2. Download the file
            fileDownloader.downloadFile(projectId, _this.onProjectDownloaded.bind(_this), _this.onFailed.bind(_this))
        })
    }

    onProjectDownloaded(data: Blob, fileName: string){
        this.triggerEvent(AnimationLoaderEvents.DOWNLOADED)
        Logger.info("Project:" + this.projectId + " downloaded!")

        this.triggerEvent(AnimationLoaderEvents.LOADBEGIN)
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
                        this.triggerEvent(AnimationLoaderEvents.LOADED)
                        Logger.info("File successfully loaded:" + storeMemoryFile)
                    }
                    else{
                        this.triggerEvent(AnimationLoaderEvents.LOADFAILED)
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
        this.triggerEvent(AnimationLoaderEvents.DOWNLOADFAILED)
    }
}

let animationLoader = new AnimationLoader()

export {animationLoader, AnimationLoaderEvents}