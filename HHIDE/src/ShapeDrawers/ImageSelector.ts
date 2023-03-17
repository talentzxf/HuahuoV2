import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {fileLoader} from "../SceneView/FileLoader";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {huahuoEngine} from "hhenginejs";

class ImageSelector extends BaseShapeDrawer{
    name = "Image"
    imgClass = "far fa-file-image"

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);

        let _this = this

        let input = document.createElement("input");
        input.type = 'file'
        input.accept = "image/*"
        input.click()

        input.onchange = e => {
            let file = (<HTMLInputElement>e.target).files[0];

            fileLoader.loadImageFile(file)
        }

        huahuoEngine.ExecuteAfterInited(()=>{
            IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)
        })
    }
}

export {ImageSelector}