import {CustomElement} from "hhcommoncomponents";
import {undoManager} from "../RedoUndo/UndoManager";

@CustomElement({
    selector: "hh-history-commands"
})
class HHHistoryCommandList extends HTMLElement{
    commandListDiv: HTMLDivElement
    connectedCallback(){
        this.commandListDiv = document.createElement("div")
        this.commandListDiv.style.height = "100px"
        this.commandListDiv.style.width = "500px"
        this.commandListDiv.style.border = "1px solid blue"
        this.commandListDiv.style.overflow = "auto"
        this.appendChild(this.commandListDiv)

        undoManager.registerListener(this.refreshCommands.bind(this))
        this.refreshCommands()
    }

    refreshCommands(){
        this.commandListDiv.innerHTML = ""
        let commands = undoManager.getCommands()
        for(let cmdIdx in commands){
            let cmd = commands[cmdIdx]
            let commandDiv = document.createElement("div")

            let commandIdSpan = document.createElement("span")
            commandIdSpan.innerText = cmdIdx.toString()
            commandIdSpan.style.paddingLeft = "30px"
            commandIdSpan.style.border = "1px solid black"

            let commandContentSpan = document.createElement("span")
            commandContentSpan.innerText = cmd.toString()
            commandContentSpan.style.border = "1px solid black"

            commandDiv.appendChild(commandIdSpan)
            commandDiv.appendChild(commandContentSpan)

            this.commandListDiv.prepend(commandDiv)
        }
    }

}

export {HHHistoryCommandList}