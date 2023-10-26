import {SVGFiles} from "./Svgs";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";

let MAXNAMELENGTH = 15

class TimelineUtils {
    createEyeIcon(timeline) {
        let _this = this
        let eyeIcon = new Image()
        eyeIcon.src = SVGFiles.eyeSvg
        eyeIcon["onIconClicked"] = function (layer) {
            let currentlyVisible = layer.GetIsVisible()
            if (currentlyVisible) { // Currently visible
                eyeIcon.src = SVGFiles.eyeSlashSvg
            } else {
                eyeIcon.src = SVGFiles.eyeSvg
            }

            layer.SetIsVisible(!currentlyVisible)
        }

        eyeIcon.onload = function () {
            timeline.reloadTracks()
        }

        return eyeIcon
    }

    setLayerNameCallback(timeline) {

        return function (layer) {
            let exitSetLayerName = false

            while (!exitSetLayerName) {
                let layerName = window.prompt("Please enter the new layer name (<=" + MAXNAMELENGTH + ")")
                if (layerName != null) {
                    if (layerName.length <= MAXNAMELENGTH) {
                        layer.SetName(layerName)
                        IDEEventBus.getInstance().emit(EventNames.LAYERINFOUPDATED, layer)
                        timeline.reloadTracks()
                        exitSetLayerName = true
                    } else {
                        window.alert("Can't input layer name >= " + MAXNAMELENGTH)
                    }
                } else {
                    exitSetLayerName = true
                }
            }
        }
    }

    initLayerTrack(timeline, layer) {
        let eyeIcon = this.createEyeIcon(timeline)
        timeline.setLayerIcons(layer, [eyeIcon])
        timeline.setLayerSetNameCallback(layer, this.setLayerNameCallback(timeline))
    }
}

let timelineUtils = new TimelineUtils()
export {timelineUtils, MAXNAMELENGTH}