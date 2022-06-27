import {Logger} from "hhcommoncomponents"
import {ImageShapeJS, huahuoEngine} from "hhenginejs"

class FileLoader{
    loadFile(file:File):boolean{
        Logger.info("Loading:" + file.name)
        const reader = new FileReader()

        reader.addEventListener('load', (e)=>{
            let img = e.target.result
            let imageShape = new ImageShapeJS()
            imageShape.setImageData(file.name, img)
            imageShape.createShape()
            imageShape.store()

            let currentLayer = huahuoEngine.GetCurrentLayer()
            currentLayer.addShape(imageShape)
        })

        reader.readAsDataURL(file)
        return false;
    }
}

let fileLoader = new FileLoader()
export {fileLoader}