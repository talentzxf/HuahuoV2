import {CustomElement} from "hhcommoncomponents";
import {SVGFiles} from "../Utilities/Svgs";
import {undoManager} from "../RedoUndo/UndoManager";

@CustomElement({
    selector: "hh-editor-tool-bar"
})
class HHEditorToolBar extends HTMLElement{
    undoButton: HTMLButtonElement
    redoButton: HTMLButtonElement

    connectedCallback(){
        this.undoButton = document.createElement("button")
        this.undoButton.style.width = "30px"
        this.undoButton.style.height = "30px"
        this.undoButton.title = i18n.t("hint.undo")
        this.undoButton.innerHTML = SVGFiles.undoBtn

        this.redoButton = document.createElement("button")
        this.redoButton.style.width = "30px"
        this.redoButton.style.height = "30px"
        this.redoButton.style.transform = "scaleX(-1)"
        this.redoButton.title = i18n.t("hint.undo")
        this.redoButton.innerHTML = SVGFiles.undoBtn

        this.undoButton.addEventListener("click", this.undo.bind(this))
        this.redoButton.addEventListener("click", this.redo.bind(this))
        this.appendChild(this.undoButton)
        this.appendChild(this.redoButton)
    }

    undo(){
        undoManager.UnDo()
    }

    redo(){
        undoManager.ReDo()
    }
}

export {HHEditorToolBar}