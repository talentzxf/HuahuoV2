// This piece of code is almost the same as LineDrawer, maybe we should extract a common base class??
import {BaseShapeDrawer} from "./BaseShapeDrawer";

class TextDrawer extends BaseShapeDrawer{
    name = "Text"
    imgClass = "far fa-edit"

    textInput: HTMLTextAreaElement

    pendingText: string
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

            this.textInput.addEventListener("focus", this.onGetFocus.bind(this))

            this.textInput.addEventListener("focusout", this.onLossFocus.bind(this))
        }

        this.textInput.style.display = "block"
        this.textInput.style.left = evt.clientX + "px"
        this.textInput.style.top = evt.clientY + "px"

        let _this = this
        window.setTimeout(()=>{
            _this.textInput.focus()
        }, 0)
    }

    onGetFocus(e){
        console.log("OnGetFocus")
        e.preventDefault()
        e.stopPropagation()
    }

    onLossFocus(e){
        console.log("OnLossFocus")
        e.preventDefault()
        e.stopPropagation()
        this.textInput.style.display = "none"
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);
    }
}

export {TextDrawer}