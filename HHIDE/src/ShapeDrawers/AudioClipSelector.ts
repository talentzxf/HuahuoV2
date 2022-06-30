import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {fileLoader} from "../SceneView/FileLoader";
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {huahuoEngine} from "hhenginejs"

class AudioClipSelector extends BaseShapeDrawer{
    name = "Audio"
    imgClass = "far fa-file-audio"

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);

        let _this = this

        let input = document.createElement("input");
        input.type = 'file'
        input.accept = "audio/*"
        input.click()

        input.onchange = e => {
            let file = (<HTMLInputElement>e.target).files[0];
            fileLoader.loadAudioFile(file)
        }

        huahuoEngine.ExecuteAfterInited(()=>{
            EventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)
        })
    }
}

export {AudioClipSelector}