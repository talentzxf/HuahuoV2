import {Logger} from "hhcommoncomponents"
import {ImageShapeJS, huahuoEngine, AudioShapeJS} from "hhenginejs"

class FileLoader{
    loadImageFile(file:File):boolean{
        Logger.info("Loading:" + file.name)
        const reader = new FileReader()

        reader.addEventListener('load', (e)=>{
            let fileExtension = file.name.substring(file.name.lastIndexOf(".") + 1)
            let img = e.target.result

            let imageShape = new ImageShapeJS()
            imageShape.setData(file.name, img, fileExtension == "gif")
            imageShape.createShape()
            imageShape.store()

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
            let audioShape = new AudioShapeJS()
            audioShape.setData(file.name, audio)

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