import "/css/contextmenu.css"

import {Logger} from "./Logger"

class MenuItem {
    public itemName: string | Function;
    public onclick: EventListener;
}

class ContextMenu {
    private items: MenuItem[]
    private menuDiv: HTMLElement

    constructor() {
    }

    setItems(items: MenuItem[]) {
        this.items = items

        if (this.menuDiv != null) {
            this.menuDiv.remove()
            this.menuDiv = null
        }
    }

    constructMenuDiv() {
        if (!this.items) {
            Logger.info("Loading ... ")
            return;
        }

        if (!this.menuDiv) {
            let menuDiv = document.createElement("div")
            menuDiv.className = "contextmenu"

            this.menuDiv = menuDiv
            document.body.appendChild(menuDiv)

            let _this = this
            document.body.addEventListener("mouseup", () => {
                if (_this.menuDiv)
                    _this.menuDiv.style.display = "none"
            })

            this.menuDiv.style.zIndex = "1000"
        }

        this.menuDiv.innerHTML = "" // Remove all contents

        let ul = document.createElement("ul")
        this.items.forEach(item => {
            let li = document.createElement("li")

            let itemName = ""
            if (item.itemName instanceof Function) {
                itemName = item.itemName()
            } else {
                itemName = item.itemName
            }

            li.innerHTML = itemName
            li.addEventListener("click", item.onclick)

            ul.appendChild(li)
        })

        this.menuDiv.appendChild(ul)
    }

    onContextMenu(e: MouseEvent) {
        e.preventDefault()
        this.constructMenuDiv()
        this.menuDiv.style.display = "block"
        this.menuDiv.style.position = "fixed";
        this.menuDiv.style.left = e.clientX + "px";
        this.menuDiv.style.top = e.clientY + "px"
    }
}

export {ContextMenu}