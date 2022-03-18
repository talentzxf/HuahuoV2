import {CustomElement} from "./CustomComponent";
import {Vector2D} from "./math/Vector2D";
import {Rect2D} from "./math/Rect2D";
import {ResizeManager} from "./resize/ResizeManager";
import {HHPanel} from "./HHPanel";

@CustomElement({
    selector: "hh-hsplitter",
    template: `<div style="
                height:5px; 
                /*border: 1px solid red;*/
                background-color: lightgray;
                "></div>`,
})
class HSplitter extends HTMLElement {
    private prevPos: Vector2D;

    constructor() {
        super()
        this.addEventListener('mouseover', this.mouseOver)
        this.addEventListener('mouseout', this.mouseOut)
        this.addEventListener('mousedown', this.mouseDown)
        this.addEventListener('mouseup', this.mouseUp)
    }

    connectedCallback() {

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
            let targetPanelRect:Rect2D = Rect2D.fromDomRect(this.parentElement.getBoundingClientRect())
            let rightDown:Vector2D = targetPanelRect.getRightDown()

            let offsetY = evt.clientY - this.prevPos.Y
            rightDown.Y += offsetY
            let newHeight = targetPanelRect.height

            ResizeManager.getInstance().adjustPanelSiblingsHeight(this.parentElement as HHPanel, newHeight)

            this.prevPos = new Vector2D(evt.clientX, evt.clientY)
        }
    }

    mouseOver() {
        this.style.cursor = 'ns-resize'
    }

    mouseOut() {
        this.style.cursor = 'none'
    }
}

export {HSplitter}