import {CustomElement} from "hhcommoncomponents";
import {SVGFiles} from "../Utilities/Svgs";
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {Player} from "hhenginejs";
import {projectInfo} from "../SceneView/ProjectInfo";

@CustomElement({
    selector: "hh-player-controller"
})
class PlayerController extends HTMLElement {
    projectNameSpan: HTMLSpanElement
    playButton: HTMLButtonElement
    pauseButton: HTMLButtonElement
    stopButton: HTMLButtonElement

    // TODO: Code duplication !!
    createButton(svgFile, onClick) {
        let btn = document.createElement("button")
        btn.className = "btn btn-outline-secondary"
        btn.style.width = "40px"
        btn.style.height = "40px"
        btn.innerHTML = svgFile
        btn.addEventListener("click", onClick)
        this.appendChild(btn)
        return btn
    }

    connectedCallback() {
        this.className = "btn-group btn-group-sm"
        this.projectNameSpan = document.createElement("span")
        this.projectNameSpan.innerText = "Unnamed project"
        this.projectNameSpan.style.verticalAlign = "super"
        this.appendChild(this.projectNameSpan)

        projectInfo.addOnChangedCallback(this.projectInfoChanged.bind(this))

        this.playButton = this.createButton(SVGFiles.playBtn, this.playAnimation.bind(this))
        this.pauseButton = this.createButton(SVGFiles.pauseBtn, this.pauseAnimation.bind(this))
        this.stopButton = this.createButton(SVGFiles.stopBtn, this.stopAnimation.bind(this))

        document.addEventListener('keydown', this.onKeyEvent.bind(this));
    }

    projectInfoChanged() {
        if (projectInfo.inited) {
            this.projectNameSpan.innerText = projectInfo.name
        }
    }

    onKeyEvent(evt: KeyboardEvent) {
        if (evt.key == "Enter" && evt.ctrlKey) { // Ctrl+Enter

            let player: Player = sceneViewManager.getFocusedViewAnimationPlayer()

            if (!player.isPlaying) {
                player.startPlay()
            } else {
                player.stopPlay()
            }

            evt.preventDefault()
        }
    }

    playAnimation() {
        let player: Player = sceneViewManager.getFocusedViewAnimationPlayer()
        player.startPlay()
    }

    pauseAnimation() {
        let player: Player = sceneViewManager.getFocusedViewAnimationPlayer()
        player.stopPlay()
    }

    stopAnimation() {
        let player: Player = sceneViewManager.getFocusedViewAnimationPlayer()
        player.stopPlay()
        player.setFrameId(0) // Reset to frame 0
    }
}

export {PlayerController}