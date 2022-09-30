import {CustomElement} from "hhcommoncomponents";
import {HHTitle} from "./HHTitle";
import {HHContent} from "./HHContent";

@CustomElement({
    selector: "hh-sidebar",
})
class HHSideBar extends HTMLElement{
    titleBar:HHTitle

    isMoving: boolean = false
    startMouseX: number = -1
    startMouseY: number = -1
    startPosX: number = -1
    startPosY: number = -1

    connectedCallback(){
        let content = this.querySelector("hh-content") as HHContent
        let title = content.getAttribute("title") || "No Title"

        this.titleBar = document.createElement("hh-title") as HHTitle
        this.titleBar.appendChild(content)
        this.titleBar.innerHTML = title
        this.titleBar.setContent(content)
        this.insertBefore(this.titleBar, null)
        this.style.background = "lightgray"
    }
}

export {HHSideBar}