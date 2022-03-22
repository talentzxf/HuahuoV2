import {CustomElement} from "./CustomComponent";
import {DomHelper} from "./utilities/DomHelper";
import {HHSplitter} from "./HHSplitter";

@CustomElement({
    selector: "hh-container",
})
class HHContainer extends HTMLElement {

    private get direction():string{
        return this.getAttribute("direction")
    }

    private get size():string{
        return this.getAttribute("size")
    }

    private get hasSplitter():boolean{
        let hasSplitter = this.getAttribute("hasSplitter")
        if(!hasSplitter || hasSplitter == "true")
            return true
        if(hasSplitter == "false")
            return false
        throw "hasSplitter can either be true or false"
    }

    private get isColumn():boolean{
        let dir:string = this.direction;
        if(dir == null || dir == "column") return true
        if(dir == "row") return false
        throw "Unknown direction, should be: column or row."
    }

    constructor() {
        super();
    }

    updateDirection(){

    }

    connectedCallback(){
        this.style.display = "flex"

        if(this.size != "fit-content"){
            this.style.width = "100%"
            this.style.height = "100%"
        }

        this.style.flexDirection = this.isColumn?"column":"row"

        if(this.hasSplitter){
            let nextElement = this.nextElementSibling
            if(nextElement != null && nextElement.nodeName.toLowerCase() == "hh-splitter")
                return

            let nextSibling = DomHelper.getNextSiblingElementByName(this, ["hh-container"])
            if(nextSibling){
                let splitter = document.createElement("hh-splitter")
                splitter.setAttribute("direction", this.parentElement.style.flexDirection)
                this.parentElement.insertBefore(splitter, nextSibling)
            }
        }
    }

    attributeChangedCallback(name:String, oldValue:any, newValue:any){
        if(name.toLowerCase() == "direction"){
            this.style.flexDirection = newValue
        }
    }

}

export {HHContainer}