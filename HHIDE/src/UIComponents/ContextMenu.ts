import "/css/contextmenu.css"

class MenuItem{
    public itemName: string;
    public onclick: EventListener;
}

class ContextMenu
{
    private items: MenuItem[]
    private menuDiv: HTMLElement

    constructor() {
    }

    setItems(items:MenuItem[]){
        this.items = items
    }

    constructMenuDiv(){
        if(!this.items){
            console.log("Loading ... ")
            return;
        }

        if(!this.menuDiv){
            let menuDiv = document.createElement("div")
            menuDiv.className = "contextmenu"

            let ul = document.createElement("ul")
            this.items.forEach(item=>{
                let li = document.createElement("li")
                li.innerHTML = item.itemName
                li.addEventListener("click", item.onclick)

                ul.appendChild(li)
            })

            menuDiv.appendChild(ul)

            this.menuDiv = menuDiv
            document.body.appendChild(menuDiv)

            document.body.addEventListener("mouseup", ()=>{
                this.menuDiv.style.display = "none"
            })
        }
    }

    onContextMenu(e:MouseEvent){
        e.preventDefault()
        this.constructMenuDiv()
        this.menuDiv.style.display = "block"
        this.menuDiv.style.position = "fixed";
        this.menuDiv.style.left = e.clientX + "px";
        this.menuDiv.style.top = e.clientY + "px"
    }
}

export {ContextMenu}