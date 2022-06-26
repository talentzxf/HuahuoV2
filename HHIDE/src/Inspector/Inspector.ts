import {CustomElement, Logger, PropertySheet} from "hhcommoncomponents"
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {BasePropertyDesc, GetPropertyDivGenerator} from "./BasePropertyDivGenerator"
import "./PropertyTypes"

@CustomElement({
    selector: "hh-inspector"
})
class Inspector extends HTMLElement{
    contentDiv:HTMLElement;
    propertyDescArray: Array<BasePropertyDesc> = new Array<BasePropertyDesc>()

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

    clearCurrentProperties(){
        this.contentDiv.innerHTML = ""

        for(let propertyDesc of this.propertyDescArray){
            propertyDesc.clear()
        }

        this.propertyDescArray = new Array<BasePropertyDesc>()
    }

    onItemSelected(propertySheet: PropertySheet){
        Logger.info("Selected something")
        this.clearCurrentProperties()

        let properties = propertySheet.getProperties()
        for(let property of properties){
            let divGenerator = GetPropertyDivGenerator(property.type)
            let propertyDesc = divGenerator.generatePropertyDesc(property)

            let propertyDiv = document.createElement("div")
            propertyDiv.style.flexDirection = divGenerator.flexDirection()
            propertyDiv.style.display = "flex"
            propertyDiv.style.width = "100%"

            propertyDiv.appendChild(propertyDesc.getTitleDiv())
            propertyDiv.appendChild(propertyDesc.getContentDiv())
            this.contentDiv.appendChild(propertyDiv)

            this.propertyDescArray.push(propertyDesc)
        }
    }
}

export {Inspector}