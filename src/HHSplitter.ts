import {CustomElement} from "./CustomComponent";
import {Vector2D} from "./math/Vector2D";
import {ResizeManager} from "./resize/ResizeManager";
import {HHPanel} from "./HHPanel";

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
        console.log("HHSplitter created!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
        this.addEventListener('mouseover', this.mouseOver)
        this.addEventListener('mouseout', this.mouseOut)
        this.addEventListener('mousedown', this.mouseDown)
        this.addEventListener('mouseup', this.mouseUp)
    }

    connectedCallback(){
        let spliterDiv = this.querySelector("div")
        spliterDiv.style.width = this.isColumn? '100%':'5px';
        spliterDiv.style.height = this.isColumn? '5px':'100%'
    }

    private get isColumn(): boolean{
        return this.getAttribute("direction") == "column"
    }

    mouseUp() {
        document.onmousemove = null
        this.prevPos = null
    }

    mouseDown(evt: MouseEvent) {
        document.onmousemove = this.mouseMove.bind(this)
        this.prevPos = new Vector2D(evt.clientX, evt.clientY)
    }

    mouseMove(evt: MouseEvent) {
        if (evt.buttons == 1) {
            let offset = -1
            if(this.isColumn)
                offset = evt.clientY - this.prevPos.Y
            else
                offset = evt.clientX - this.prevPos.X

            ResizeManager.getInstance().adjustSiblingsSize(this, offset, this.isColumn, this.getAttribute('siblingElementName'))

            this.prevPos = new Vector2D(evt.clientX, evt.clientY)
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