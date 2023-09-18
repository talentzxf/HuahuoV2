import * as React from "react"
import {projectInfo} from "../SceneView/ProjectInfo";
import {imgButton} from "../UIComponents/MainMenuX";
import {SVGFiles} from "../Utilities/Svgs";
import {sceneViewManager} from "../SceneView/SceneViewManager";

type PlayerControllerState = {
    projectName: string
    isPlaying: boolean
    isPaused: boolean
}

class PlayerControllerX extends React.Component<any, PlayerControllerState> {
    state: PlayerControllerState = {
        projectName: "Unknown Project",
        isPlaying: false,
        isPaused: false
    }

    componentDidMount() {
        projectInfo.addOnChangedCallback(this.projectInfoChanged.bind(this))
    }

    projectInfoChanged() {
        if (projectInfo.inited) {
            this.state.projectName = projectInfo.name
        }
    }

    syncStateWithPlayer() {
        let player = sceneViewManager.getFocusedViewAnimationPlayer()
        this.state.isPaused = player.isPaused
        this.state.isPlaying = player.isPlaying
    }

    render() {
        let player = sceneViewManager.getFocusedViewAnimationPlayer()

        this.syncStateWithPlayer()

        let playButton = imgButton(SVGFiles.playBtn, i18n.t("playerController.Play"), () => {
            player.startPlay()
            this.syncStateWithPlayer()
            this.setState(this.state)
        })
        let stopButton = imgButton(SVGFiles.stopBtn, i18n.t("playerController.Stop"), () => {
            sceneViewManager.getFocusedViewAnimationPlayer().stopPlay()
            this.syncStateWithPlayer()
            this.setState(this.state)
        })

        let pauseButton = imgButton(SVGFiles.pauseBtn, i18n.t("playerController.Pause"), () => {
            sceneViewManager.getFocusedViewAnimationPlayer().pausePlay()
            this.syncStateWithPlayer()
            this.setState(this.state)
        })

        return (
            <>
                <span>{this.state.projectName}</span>
                <div>
                    {this.state.isPlaying ? pauseButton : playButton}
                    {this.state.isPlaying ? (this.state.isPaused ? playButton : stopButton) : null}
                </div>
            </>
        )
    }
}

export {PlayerControllerX}