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
            this.setState(this.state)
        }
    }

    syncStateWithPlayer() {
        let player = sceneViewManager.getFocusedViewAnimationPlayer()
        this.state.isPaused = player?.isPaused
        this.state.isPlaying = player?.isPlaying
    }

    render() {
        let player = sceneViewManager.getFocusedViewAnimationPlayer()
        this.syncStateWithPlayer()

        let playButton = imgButton(SVGFiles.playBtn, i18n.t("playerController.Play"), () => {
            let player = sceneViewManager.getFocusedViewAnimationPlayer()
            player?.startPlay()
            this.syncStateWithPlayer()
            this.setState(this.state)
        })
        let stopButton = imgButton(SVGFiles.stopBtn, i18n.t("playerController.Stop"), () => {
            let player = sceneViewManager.getFocusedViewAnimationPlayer()
            player?.stopPlay()
            this.syncStateWithPlayer()
            this.setState(this.state)
        })

        let pauseButton = imgButton(SVGFiles.pauseBtn, i18n.t("playerController.Pause"), () => {
            let player = sceneViewManager.getFocusedViewAnimationPlayer()
            player?.pausePlay()
            this.syncStateWithPlayer()
            this.setState(this.state)
        })

        return (
            <div className="flex flex-row-reverse">
                <div className="flex flex-row w-[50%]">
                    {this.state.isPlaying ? pauseButton : playButton}
                    {this.state.isPlaying ? (this.state.isPaused ? playButton : stopButton) : null}
                </div>
                <span className="w-[50%]">{this.state.projectName}</span>
            </div>
        )
    }
}

export {PlayerControllerX}