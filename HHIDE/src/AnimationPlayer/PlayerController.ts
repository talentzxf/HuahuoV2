import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-player-controller"
})
class PlayerController extends HTMLHtmlElement{
    playButton: HTMLButtonElement
    pauseButton: HTMLButtonElement
    stopButton: HTMLButtonElement

    connectedCallBack(){
        this.playButton = document.createElement("button")
        this.pauseButton = document.createElement("button")
        this.stopButton = document.createElement("button")

        this.appendChild(this.playButton)
        this.appendChild(this.pauseButton)
        this.appendChild(this.stopButton)
    }
}

export {PlayerController}