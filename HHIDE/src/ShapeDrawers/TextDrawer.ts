// This piece of code is almost the same as LineDrawer, maybe we should extract a common base class??
import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {TextShapeJS, huahuoEngine} from "hhenginejs"
import {Vector2} from "hhcommoncomponents";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {EditorShapeProxy} from "./EditorShapeProxy";

class TextDrawer extends BaseShapeDrawer{
    name = "Text"
    imgClass = "far fa-edit"

    textInput: HTMLTextAreaElement

    textShape: TextShapeJS
    textPos:Vector2 = new Vector2
    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);

        canvas.style.cursor = "text"
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        if(!this.textInput){
            // Create a text input here.
            this.textInput = document.createElement("textarea")
            this.textInput.style.position = "absolute"
            document.body.appendChild(this.textInput)

            this.textInput.addEventListener("input", this.onTextChanged.bind(this))
            this.textInput.addEventListener("focusout", this.onLossFocus.bind(this))
        }

        this.textInput.style.display = "block"
        this.textInput.style.left = evt.clientX + "px"
        this.textInput.style.top = evt.clientY + "px"

        this.textPos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

        let _this = this
        window.setTimeout(()=>{
            _this.textInput.focus()
        }, 0)
    }

    onLossFocus(e){
        this.textInput.style.display = "none"
        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            _this.isDrawing = false
            IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)

            _this.addShapeToCurrentLayer(_this.textShape)

            this.textShape = EditorShapeProxy.CreateProxy(new TextShapeJS())
        })

        this.textInput.value = ""
    }

    onTextChanged(){
        let curText:string = this.textInput.value

        if(!this.textShape)
            this.textShape = EditorShapeProxy.CreateProxy(new TextShapeJS());

        this.textShape.setTextWithPosition(curText, this.textPos)
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);
    }
}

export {TextDrawer}