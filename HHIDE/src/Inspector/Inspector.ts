import {CustomElement, Logger, PropertySheet} from "hhcommoncomponents"
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {GeneratePropertyDiv, GetPropertyFlexDirection} from "./BasePropertyDivGenerator"
import "./PropertyTypes"

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
        div.style.overflowY = "scroll"
        this.appendChild(div)

        this.contentDiv = document.createElement("div")
        this.contentDiv.style.width="100%"
        div.appendChild(this.contentDiv)

        EventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))
    }

    onItemSelected(propertySheet: PropertySheet){
        Logger.info("Selected something")

        this.contentDiv.innerHTML = ""
        let properties = propertySheet.getProperties()
        for(let property of properties){
            let propertyDiv = document.createElement("div")
            propertyDiv.style.flexDirection = GetPropertyFlexDirection(property.type)
            propertyDiv.style.display = "flex"
            propertyDiv.style.width = "100%"
            let keyDiv = document.createElement("div")
            keyDiv.innerText = property.key
            propertyDiv.appendChild(keyDiv)
            propertyDiv.appendChild(GeneratePropertyDiv(property.type, property))
            this.contentDiv.appendChild(propertyDiv)
        }
    }
}

export {Inspector}