import {fileDownloader} from "../RestApis/FileDownloader";
import {Logger} from "hhcommoncomponents";
import {PlayerView} from "./PlayerView";

declare var Module: any;

class AnimationLoader{
    projectId:string = "unknownProject"
    loadAnimation(projectId: string){
        this.projectId = projectId

        // 1. Wait for the PlayerView ready.

        let playerView:PlayerView = document.querySelector("hh-player")

        let _this = this
        playerView.executeAfterInit(function(){
            // 2. Download the file
            fileDownloader.downloadFile(projectId, _this.onProjectDownloaded.bind(_this), _this.onFailed.bind(_this))
        })
    }

    onProjectDownloaded(data: Blob, fileName: string){
        Logger.info("Project:" + this.projectId + " downloaded!")

        Promise.resolve( data.arrayBuffer() ).then(
            (fileContent)=>{
                let storeMemoryFile = "mem://" + fileName;
                let fileSize = data.size;
                let memoryFileContent = Module.createMemFile(storeMemoryFile, fileSize);
                for (let i = 0; i < fileSize; i++) { // Copy to the file, byte by byte
                    memoryFileContent[i] = fileContent[i];
                }

                let result = Module.LoadFileCompletely(storeMemoryFile);
                if (result == 0)
                    console.log("Can't load file: " + storeMemoryFile)
            }
        ).catch((error)=>{
            Logger.error("Error happened:" + error)
        })
    }

    onFailed(status:string, msg: string){
        Logger.error("Can't download project:" + this.projectId + " Status:" + status + " Msg:" + msg)
    }
}

let animationLoader = new AnimationLoader()

export {animationLoader}