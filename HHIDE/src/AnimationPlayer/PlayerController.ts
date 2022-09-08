import {CustomElement} from "hhcommoncomponents";
import {SVGFiles} from "../Utilities/Svgs";

@CustomElement({
    selector: "hh-player-controller"
})
class PlayerController extends HTMLElement{
    playButton: HTMLButtonElement
    pauseButton: HTMLButtonElement
    stopButton: HTMLButtonElement

    connectedCallback() {
        this.playButton = document.createElement("button")
        this.playButton.innerText = "Play"
        this.playButton.style.width = "30px"
        this.playButton.style.height = "30px"
        this.playButton.innerHTML = SVGFiles.playBtn
        this.pauseButton = document.createElement("button")
        this.pauseButton.style.width = "30px"
        this.pauseButton.style.height = "30px"
        this.pauseButton.innerHTML = SVGFiles.pauseBtn
        this.stopButton = document.createElement("button")
        this.stopButton.style.width = "30px"
        this.stopButton.style.height = "30px"
        this.stopButton.innerHTML = SVGFiles.stopBtn

        this.appendChild(this.playButton)
        this.appendChild(this.pauseButton)
        this.appendChild(this.stopButton)
    }
}

export {PlayerController}