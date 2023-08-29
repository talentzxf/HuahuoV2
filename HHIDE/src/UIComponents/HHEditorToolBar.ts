import {CustomElement} from "hhcommoncomponents";
import {SVGFiles} from "../Utilities/Svgs";
import {undoManager} from "../RedoUndo/UndoManager";

@CustomElement({
    selector: "hh-editor-tool-bar"
})
class HHEditorToolBar extends HTMLElement {
    undoButton: HTMLButtonElement
    redoButton: HTMLButtonElement
    keyboardButton: HTMLButtonElement // Press to configure short cuts

    createButton(svgFile, title, onClick) {
        let btn = document.createElement("button")
        btn.className =  "btn btn-outline-secondary"
        btn.style.width = "40px"
        btn.style.height = "40px"
        btn.title = title
        btn.innerHTML = svgFile
        btn.addEventListener("click", onClick)
        this.appendChild(btn)
        return btn
    }

    connectedCallback() {
        this.className = "btn-group btn-group-sm"
        i18n.ExecuteAfterInited(function () {
            this.undoButton = this.createButton(SVGFiles.undoBtn, i18n.t("hint.undo"), this.undo.bind(this))

            this.redoButton = this.createButton(SVGFiles.undoBtn, i18n.t("hint.redo"), this.redo.bind(this))
            this.redoButton.style.transform = "scaleX(-1)" // Flip

            this.keyboardButton = this.createButton(SVGFiles.keyboardBtn, i18n.t("hint.shortcuts"), this.configureShortcuts.bind(this))
        }.bind(this))
    }

    undo() {
        undoManager.UnDo()
    }

    redo() {
        undoManager.ReDo()
    }

    configureShortcuts() {

    }
}

export {HHEditorToolBar}