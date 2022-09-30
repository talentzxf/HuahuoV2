import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-sidebar",
})
class HHSideBar extends HTMLElement{
    titleBar:HTMLElement

    isMoving: boolean = false

    connectedCallback(){
        this.titleBar = document.createElement("div")
        this.titleBar.onmousedown = this.onTitleMouseDown.bind(this)
        this.titleBar.onmousemove = this.onTitleMouseMove.bind(this)
        this.titleBar.onmouseup = this.onTitleMouseUp.bind(this)

        this.titleBar.innerText = this.getAttribute("title") || "No title"
        this.style.position = "absolute"

        this.insertBefore(this.titleBar, null)
        this.style.background = "lightgray"
    }

    onTitleMouseDown(evt: MouseEvent){
        if(evt.)
    }

    onTitleMouseMove(evt: MouseEvent){

    }

    onTitleMouseUp(evt: MouseEvent){

    }
}

export {HHSideBar}