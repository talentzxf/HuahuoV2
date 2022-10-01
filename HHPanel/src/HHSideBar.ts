import {CustomElement} from "hhcommoncomponents";
import {HHTitle} from "./HHTitle";
import {HHContent} from "./HHContent";
import {Vector2D} from "./math/Vector2D";
import {TabMover} from "./draggable/TabMover";
import {OccupiedTitleManager} from "./draggable/OccupiedTitleManager";

@CustomElement({
    selector: "hh-sidebar-content",
})
class SideBarContent extends HTMLElement{

}

@CustomElement({
    selector: "hh-sidebar",
})
class HHSideBar extends HTMLElement{
    titleBar: HTMLDivElement

    isMoving: boolean = false
    startMoving: boolean = false
    startPos: Vector2D
    startElePos: Vector2D

    connectedCallback(){
        let content = this.querySelector("hh-sidebar-content") as HHContent
        let title = content.getAttribute("title") || "No Title"

        this.titleBar = document.createElement("div") as HTMLDivElement
        this.titleBar.innerHTML = title
        this.insertBefore(this.titleBar, content)
        this.titleBar.style.background = "lightgray"

        this.style.position = "absolute"
        this.style.position = "absolute"
        this.style.background = "white"

        this.style.border = "1px solid black"

        this.titleBar.onmousedown = this.onTitleMouseDown.bind(this)
    }

    // TODO: Avoid duplication with HHTitle
    onTitleMouseDown(evt: MouseEvent){
        this.startPos = new Vector2D(evt.clientX, evt.clientY)
        this.startMoving = true
        this.isMoving = false
        this.startElePos = new Vector2D(this.offsetLeft, this.offsetTop);
        document.onmousemove = this.mouseMove.bind(this)
        document.onmouseup = this.mouseUp.bind(this)
    }

    mouseMove(evt:MouseEvent) {
        if (evt.buttons == 1) {
            if (this.startMoving && !this.startPos.equals(evt.clientX, evt.clientY)) {
                this.isMoving = true
            }

            if (this.isMoving) {
                let offsetX = evt.clientX - this.startPos.X;
                let offsetY = evt.clientY - this.startPos.Y;

                let targetX = this.startElePos.X + offsetX;
                let targetY = this.startElePos.Y + offsetY;

                this.style.left = targetX + "px"
                this.style.top = targetY + "px"
            }
        } else {
            this.endMoving()
        }
    }

    mouseUp(evt:MouseEvent) {
        this.endMoving()
    }

    endMoving() {
        this.startMoving = false
        this.isMoving = false
        document.onmousemove = null
        document.onmouseup = null
    }

}

export {HHSideBar, SideBarContent}