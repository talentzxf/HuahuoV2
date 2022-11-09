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
    shapePropertyDivMapping: Map<any, HTMLElement> = new Map()

    connectedCallback() {
        let parentPanel = findParentPanel(this)
        let parentHeight = parentPanel?parentPanel.clientHeight: 300;

        this.contentScrollerDiv = document.createElement("div")
        this.contentScrollerDiv .style.width = "100%"
        this.contentScrollerDiv .style.height = parentHeight + "px"
        this.contentScrollerDiv .style.overflowY = "auto"
        this.contentScrollerDiv.style.overflowX = "auto"
        this.contentScrollerDiv.style.resize = "both"
        this.appendChild(this.contentScrollerDiv )

        EventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))
        EventBus.getInstance().on(EventNames.UNSELECTOBJECTS, this.unselectObjects.bind(this))

        findParentSideBar(this).hide()
    }

    unselectObjects(){
        findParentSideBar(this).hide()
    }

    onItemSelected(propertySheet: PropertySheet, targetObj: any){

        findParentSideBar(this).show()

        let parentContainer = findParentSideBar(this)
        let titleBarHeight = parentContainer.querySelector(".title_tabs").offsetHeight
        let parentHeight = parentContainer.clientHeight - titleBarHeight;
        this.contentScrollerDiv.style.height = (parentHeight) + "px"

        while(this.contentScrollerDiv.firstChild){
            this.contentScrollerDiv.removeChild(this.contentScrollerDiv.firstChild)
        }

        if(this.shapePropertyDivMapping.has(targetObj)){
            let contentDiv = this.shapePropertyDivMapping.get(targetObj)
            this.contentScrollerDiv.appendChild(contentDiv)
        }else{
            Logger.info("Selected something")

            let contentDiv = document.createElement("div")
            contentDiv.style.width="100%"
            this.contentScrollerDiv.appendChild(contentDiv)

            let properties = propertySheet.getProperties()
            for(let property of properties){
                let divGenerator = GetPropertyDivGenerator(property.type)
                let propertyDesc = divGenerator.generatePropertyDesc(property)

                let propertyDiv = GenerateDiv(divGenerator, propertyDesc)
                contentDiv.appendChild(propertyDiv)
            }

            this.shapePropertyDivMapping.set(targetObj, contentDiv)
        }
    }
}

export {Inspector}