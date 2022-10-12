import {CustomElement, Logger, PropertySheet} from "hhcommoncomponents"
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {BasePropertyDesc, GenerateDiv, GetPropertyDivGenerator} from "./BasePropertyDivGenerator"
import "./PropertyTypes"
import {findParentSideBar, findParentPanel} from "hhpanel";

@CustomElement({
    selector: "hh-inspector"
})
class Inspector extends HTMLElement{
    contentScrollerDiv:HTMLDivElement;
    contentDiv:HTMLElement;
    propertyDescArray: Array<BasePropertyDesc> = new Array<BasePropertyDesc>()

    connectedCallback() {
        let parentPanel = findParentPanel(this)
        let parentHeight = parentPanel?parentPanel.clientHeight: 300;

        this.contentScrollerDiv = document.createElement("div")
        this.contentScrollerDiv .style.width = "100%"
        this.contentScrollerDiv .style.height = parentHeight + "px"
        this.contentScrollerDiv .style.overflowY = "auto"
        this.contentScrollerDiv.style.overflowX = "clip"
        this.appendChild(this.contentScrollerDiv )

        this.contentDiv = document.createElement("div")
        this.contentDiv.style.width="100%"
        this.contentScrollerDiv.appendChild(this.contentDiv)

        EventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))
        EventBus.getInstance().on(EventNames.UNSELECTOBJECTS, this.unselectObjects.bind(this))

        findParentSideBar(this).hide()
    }

    unselectObjects(){
        findParentSideBar(this).hide()
    }

    clearCurrentProperties(){
        this.contentDiv.innerHTML = ""

        for(let propertyDesc of this.propertyDescArray){
            propertyDesc.clear()
        }

        this.propertyDescArray = new Array<BasePropertyDesc>()
    }

    onItemSelected(propertySheet: PropertySheet){

        findParentSideBar(this).show()

        let parentContainer = findParentSideBar(this)
        let titleBarHeight = parentContainer.querySelector(".title_tabs").offsetHeight
        let parentHeight = parentContainer.clientHeight - titleBarHeight;
        this.contentScrollerDiv.style.height = (parentHeight) + "px"

        Logger.info("Selected something")
        this.clearCurrentProperties()

        let properties = propertySheet.getProperties()
        for(let property of properties){
            let divGenerator = GetPropertyDivGenerator(property.type)
            let propertyDesc = divGenerator.generatePropertyDesc(property)

            let propertyDiv = GenerateDiv(divGenerator, propertyDesc)
            this.contentDiv.appendChild(propertyDiv)

            this.propertyDescArray.push(propertyDesc)
        }
    }
}

export {Inspector}