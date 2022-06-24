import {CustomElement, Logger, PropertySheet} from "hhcommoncomponents"
import {EventBus, EventNames} from "../Events/GlobalEvents";

@CustomElement({
    selector: "hh-inspector"
})
class Inspector extends HTMLElement{
    contentDiv:HTMLElement;
    connectedCallback() {
        let parentHeight = this.parentElement.parentElement.clientHeight;

        let div = document.createElement("div")
        div.style.width = "100%"
        div.style.height = parentHeight + "px"
        div.style.overflow = "scroll"
        this.appendChild(div)

        this.contentDiv = document.createElement("div")
        this.contentDiv.style.width="100px"
        this.contentDiv.style.height="10000px"
        div.appendChild(this.contentDiv)

        EventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))
    }

    onItemSelected(property: PropertySheet){

    }
}

export {Inspector}