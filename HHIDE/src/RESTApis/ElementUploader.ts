import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {gzipSync} from "fflate";
import {api} from "./RestApi";
import {HHToast} from "hhcommoncomponents";
import {SnapshotUtils} from "../Utilities/SnapshotUtils";

declare var Module: any

function randomElementFileName(length = 10) {
    let result = '';
    const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    const charactersLength = characters.length;
    let counter = 0;
    while (counter < length) {
        result += characters.charAt(Math.floor(Math.random() * charactersLength));
        counter += 1;
    }
    return result;
}

class ElementUploader {

    getStoreData(storeId, elementName){
        let uint8Array = Module.writeObjectStoreInMemoryFile(storeId)
        let compressedFileContent = gzipSync(uint8Array, {filename: elementName + ".ele"})

        return new Blob([compressedFileContent], {type:"application/octet-stream"})
    }

    @NeedLogin()
    uploadStore(storeId, elementName, isShareable, isEditable) {
        let elementBlob = this.getStoreData(storeId, elementName)

        api.uploadElement(elementBlob, elementName, storeId, isShareable, isEditable).then((response)=>{
            if(response && response["data"]){
                let fileId = response["data"]["binaryFileDB"]["id"]

                SnapshotUtils.takeSnapShotForStore(storeId).then((blob)=>{
                    let uploadCoverpagePromise = api.uploadProjectCoverPage(fileId, blob, elementName + ".png", true)
                    uploadCoverpagePromise.then((response)=>{
                        HHToast.info("Element upload succeeded!")
                    })
                })
            }
        })
    }
}

let elementUploader = new ElementUploader()
export {elementUploader}