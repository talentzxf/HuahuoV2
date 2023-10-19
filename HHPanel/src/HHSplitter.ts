import {CustomElement} from "hhcommoncomponents";
import {Vector2D} from "./math/Vector2D";
import {ResizeManager} from "./resize/ResizeManager";

@CustomElement({
    selector: "hh-splitter",
    template: `<div style="
                /*border: 1px solid red;*/
                background-color: lightgray;
                "></div>`,
})
class HHSplitter extends HTMLElement {
    private prevPos: Vector2D;

    constructor() {
        super()
        this.addEventListener('mouseover', this.mouseOver)
        this.addEventListener('mouseout', this.mouseOut)
        this.addEventListener('mousedown', this.mouseDown)
        this.addEventListener('mouseup', this.mouseUp)
    }

    resetSize(){
        let spliterDiv = this.querySelector("div")
        spliterDiv.style.width = this.isColumn? '100%':'5px';
        spliterDiv.style.height = this.isColumn? '5px':'100%'
    }
    connectedCallback(){
        this.resetSize()
    }

    private get isColumn(): boolean{
        return this.getAttribute("direction") == "column"
    }

    private attributeChangedCallback(name: String, oldValue: any, newValue: any){
        if(name == "direction"){
            this.resetSize()
        }
    }

    mouseUp() {
        document.onmousemove = null
        this.prevPos = null
        this.isMoving = false
    }

    isMoving = false

    mouseDown(evt: MouseEvent) {
        document.onmousemove = this.mouseMove.bind(this)
        this.prevPos = new Vector2D(evt.clientX, evt.clientY)

        this.isMoving = true
    }

    mouseMove(evt: MouseEvent) {
        if (evt.buttons == 1 && this.isMoving) {
            let offset = -1
            if(this.isColumn)
                offset = evt.clientY - this.prevPos.Y
            else
                offset = evt.clientX - this.prevPos.X

            ResizeManager.getInstance().adjustSiblingsSize(this, offset, this.isColumn)

            this.prevPos = new Vector2D(evt.clientX, evt.clientY)

            evt.preventDefault()
            evt.stopPropagation()
        } else {
            this.mouseUp()
        }
    }

    mouseOver() {
        if (this.isColumn)
            this.style.cursor = 'ns-resize'
        else
            this.style.cursor = 'ew-resize'
    }

    mouseOut() {
        this.style.cursor = 'none'
    }
}

export {HHSplitter}