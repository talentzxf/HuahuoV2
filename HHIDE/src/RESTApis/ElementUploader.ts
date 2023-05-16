import {NeedLogin} from "../Identity/NeedLoginAnnotation";
import {gzipSync} from "fflate";
import {api} from "./RestApi";
import {huahuoEngine} from "hhenginejs";

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
    uploadStore(storeId, elementName) {
        api.createFile(elementName, true).then((response)=>{
            if(response && response.data && response.data.succeeded){
                let fileId = response.data.fileId
                huahuoEngine.setStoreFileId(storeId, fileId)

                let storeData = this.getStoreData(storeId, elementName)
                api.uploadProject(storeData, elementName, true)
            }
        })
    }
}

let elementUploader = new ElementUploader()
export {elementUploader}