import * as React from "react"
import {projectInfo} from "../SceneView/ProjectInfo";
import {imgButton} from "../UIComponents/MainMenuX";
import {SVGFiles} from "../Utilities/Svgs";
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {huahuoEngine} from "hhenginejs";

type PlayerControllerState = {
    projectName: string
    isPlaying: boolean
    isPaused: boolean
}

class PlayerControllerX extends React.Component<any, PlayerControllerState> {
    state: PlayerControllerState = {
        projectName: i18n.t("UnnamedProject"),
        isPlaying: false,
        isPaused: false
    }

    componentDidMount() {
        IDEEventBus.getInstance().on(EventNames.PROJECTINFOUPDATED, this.projectInfoChanged.bind(this))
    }

    projectInfoChanged() {
        this.state.projectName = projectInfo.name
        this.setState(this.state)
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
            <div className="flex flex-row">
                <span className="w-[50%] flex flex-col">
                    <span>{this.state.projectName}</span>
                    <span>
                        {
                            "Engine-Version:" + huahuoEngine.getEngineVersion()
                        }
                    </span>
                </span>
                <div className="flex flex-row w-[50%]">
                    {this.state.isPlaying ? pauseButton : playButton}
                    {this.state.isPlaying ? (this.state.isPaused ? playButton : stopButton) : null}
                </div>
            </div>
        )
    }
}

export {PlayerControllerX}