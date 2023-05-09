import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {gzipSync} from "fflate";
import {api} from "./RestApi";

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

    getStoreData(storeId){
        let elementFileName = Module.getStoreFilePath()
        let uint8Array = Module.writeObjectStoreInMemoryFile(storeId)
        let compressedFileContent = gzipSync(uint8Array, {filename: elementFileName})

        return new Blob([compressedFileContent], {type:"application/octet-stream"})
    }

    @NeedLogin()
    uploadStore(storeId) {
        let storeData = this.getStoreData(storeId)
        let fileName = Module.getStoreFilePath()
        let uploadElementPromise = api.uploadProject(storeData, fileName, true)

        return uploadElementPromise
    }
}

let elementUploader = new ElementUploader()
export {elementUploader}