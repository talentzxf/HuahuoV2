import {Logger} from "hhcommoncomponents"
import {ImageShapeJS, huahuoEngine, AudioShapeJS} from "hhenginejs"
import {dataURItoBlob, getMimeTypeFromDataURI} from "hhcommoncomponents";
let md5 = require("js-md5")

function loadBinaryDataIntoStore(fileName: string, data){
    let binaryData:Uint8Array = dataURItoBlob(data)

    let resourceMD5 = md5(binaryData)

    if(!huahuoEngine.IsBinaryResourceExist(resourceMD5))
        huahuoEngine.LoadBinaryResource(fileName, getMimeTypeFromDataURI(data), binaryData, binaryData.length)

    return resourceMD5
}

class FileLoader{
    loadImageFile(file:File):boolean{
        Logger.info("Loading:" + file.name)
        const reader = new FileReader()

        reader.addEventListener('load', (e)=>{
            let fileExtension = file.name.substring(file.name.lastIndexOf(".") + 1)
            let img = e.target.result

            let resourceMD5 = loadBinaryDataIntoStore(file.name, img)

            let imageShape = new ImageShapeJS()
            imageShape.setResourceByMD5(resourceMD5)
            imageShape.isAnimation = fileExtension == "gif"
            imageShape.createShape()

            let currentLayer = huahuoEngine.GetCurrentLayer()
            currentLayer.addShape(imageShape)
        })

        reader.readAsDataURL(file)
        return false;
    }

    loadAudioFile(file:File):boolean{
        Logger.info("Loading: " + file.name)
        const reader = new FileReader()

        reader.addEventListener("load",(e)=>{
            let audio = e.target.result

            let resourceMD5 = loadBinaryDataIntoStore(file.name, audio)

            let audioShape = new AudioShapeJS()
            audioShape.setResourceByMD5(resourceMD5)

            audioShape.createShape()
            audioShape.store()
            let currentLayer = huahuoEngine.GetCurrentLayer()
            currentLayer.addShape(audioShape)
        })

        reader.readAsDataURL(file)

        return false;
    }
}

let fileLoader = new FileLoader()
export {fileLoader}