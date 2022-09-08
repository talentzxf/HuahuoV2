import {CustomElement} from "hhcommoncomponents";
import {SVGFiles} from "../Utilities/Svgs";
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {Player} from "hhenginejs";

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

        this.playButton.onclick = this.playAnimation.bind(this)
        this.pauseButton.onclick = this.pauseAnimation.bind(this)
        this.stopButton.onclick = this.stopAnimation.bind(this)

        document.addEventListener('keydown', this.onKeyEvent.bind(this));
    }

    onKeyEvent(evt:KeyboardEvent){
            if(evt.key == "Enter" && evt.ctrlKey){ // Ctrl+Enter

                let player:Player = sceneViewManager.getFocusedViewAnimationPlayer()

                if(!player.isPlaying){
                    player.startPlay()
                }else{
                    player.stopPlay()
                }

                evt.preventDefault()
            }
    }

    playAnimation(){
        let player:Player = sceneViewManager.getFocusedViewAnimationPlayer()
        player.startPlay()
    }

    pauseAnimation(){
        let player:Player = sceneViewManager.getFocusedViewAnimationPlayer()
        player.stopPlay()
    }

    stopAnimation(){
        let player:Player = sceneViewManager.getFocusedViewAnimationPlayer()
        player.stopPlay()
        player.setFrameId(0) // Reset to frame 0
    }
}

export {PlayerController}