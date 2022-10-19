import {CustomElement} from "hhcommoncomponents";
import {undoManager} from "../RedoUndo/UndoManager";
import {findParentPanel} from "hhpanel"

@CustomElement({
    selector: "hh-history-commands"
})
class HHHistoryCommandList extends HTMLElement {
    commandListDiv: HTMLDivElement
    contentScrollerDiv: HTMLDivElement

    connectedCallback() {
        let parentPanel = findParentPanel(this)
        let parentHeight = parentPanel ? parentPanel.clientHeight : 300;

        this.contentScrollerDiv = document.createElement("div")
        this.contentScrollerDiv.style.width = "100%"
        this.contentScrollerDiv.style.height = parentHeight + "px"
        this.contentScrollerDiv.style.overflowY = "auto"
        this.contentScrollerDiv.style.overflowX = "auto"
        this.contentScrollerDiv.style.resize = "both"
        this.appendChild(this.contentScrollerDiv)


        this.commandListDiv = document.createElement("div")
        this.commandListDiv.style.height = "100%"
        this.commandListDiv.style.width = "200px"
        this.commandListDiv.style.border = "1px solid blue"
        this.commandListDiv.style.overflow = "auto"
        this.commandListDiv.style.position = "relative"
        this.contentScrollerDiv.appendChild(this.commandListDiv)

        undoManager.registerListener(this.refreshCommands.bind(this))
        this.refreshCommands()
    }

    refreshCommands() {
        this.commandListDiv.innerHTML = ""
        let commands = undoManager.getCommands()
        let currentCommandIndex = undoManager.getDisplayIndex()

        let currentSelecedCommandDiv: HTMLElement

        for (let cmdIdx in commands) {
            let cmd = commands[cmdIdx]
            let commandDiv = document.createElement("div")
            let commandIdSpan = document.createElement("span")

            let indicatorStr = (currentCommandIndex == parseInt(cmdIdx)) ? "->" : ""

            commandIdSpan.innerText = indicatorStr + cmdIdx.toString()
            commandIdSpan.style.paddingLeft = "30px"
            commandIdSpan.style.border = "1px solid black"

            let commandContentSpan = document.createElement("span")
            commandContentSpan.innerText = cmd.toString()
            commandContentSpan.style.border = "1px solid black"

            commandDiv.appendChild(commandIdSpan)
            commandDiv.appendChild(commandContentSpan)

            if (indicatorStr.length > 0) {
                currentSelecedCommandDiv = commandDiv
            }

            this.commandListDiv.prepend(commandDiv)
        }

        if (currentSelecedCommandDiv != null) {
            let yDistanceToListDivTop = currentSelecedCommandDiv.offsetTop - this.commandListDiv.scrollTop
            if (yDistanceToListDivTop > this.commandListDiv.clientHeight * 0.9 ||
                currentSelecedCommandDiv.offsetTop < this.commandListDiv.scrollTop + this.commandListDiv.clientHeight * 0.1)
                this.commandListDiv.scroll(0, currentSelecedCommandDiv.offsetTop)
        }
    }

}

export {HHHistoryCommandList}