import * as React from "react"
import {SVGFiles} from "../Utilities/Svgs";
import {undoManager} from "../RedoUndo/UndoManager";
import {imgButton} from "./MainMenuX";
import {HHToast} from "hhcommoncomponents";

class EditorToolBarX extends React.Component<any, any> {
    render() {
        return (<div className="w-[25%]">
            {imgButton(SVGFiles.undoBtn, i18n.t("hint.undo"), () => {
                undoManager.UnDo()
            })}
            {imgButton(SVGFiles.undoBtn, i18n.t("hint.redo"), () => {
                undoManager.ReDo()
            }, {
                transform: "scaleX(-1)"
            })}
            {imgButton(SVGFiles.keyboardBtn, i18n.t("hint.shortcuts"), () => {
                HHToast.warn(i18n.t("notImplemented"))
            })}
        </div>)
    }
}

export {EditorToolBarX}