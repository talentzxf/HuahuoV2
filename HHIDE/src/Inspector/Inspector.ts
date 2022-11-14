import {CustomElement, Logger, PropertySheet} from "hhcommoncomponents"
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {GenerateDiv, GetPropertyDivGenerator} from "./BasePropertyDivGenerator"
import "./PropertyTypes"
import {findParentSideBar, findParentPanel} from "hhpanel";
import {PropertyType} from "hhcommoncomponents";
import {formManager} from "../Utilities/FormManager";
import {ComponentListForm} from "../UIComponents/ComponentListForm";
import {huahuoEngine} from "hhenginejs";
import {HHRefreshableDiv} from "./InputComponents/HHRefreshableDiv";

@CustomElement({
    selector: "hh-inspector"
})
class Inspector extends HTMLElement{
    contentScrollerDiv:HTMLDivElement;
    shapePropertyDivMapping: Map<any, HHRefreshableDiv> = new Map()

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

        EventBus.getInstance().on(EventNames.COMPONENTADDED, this.componentAdded.bind(this))

        findParentSideBar(this).hide()
    }

    unselectObjects(){
        findParentSideBar(this).hide()
    }

    componentAdded(targetObj: any){
        if(this.shapePropertyDivMapping.has(targetObj)){
            this.shapePropertyDivMapping.delete(targetObj)
        }

        this.onItemSelected(targetObj.getPropertySheet(), targetObj)
    }

    createOpenCollapseButton(allComponentTitleDivs: Array<HTMLElement>){

        let collapseAllButton = document.createElement("button")
        collapseAllButton.innerText = i18n.t("inspector.CollapseAll")

        let isCollapseAll = true
        collapseAllButton.onclick = function(){
            for(let componentTitleDiv of allComponentTitleDivs){

                let currentlyCollapsed = false

                if(componentTitleDiv.getAttribute("isCollapsed") == "true"){
                    currentlyCollapsed = true
                }

                if(isCollapseAll){
                    if(!currentlyCollapsed)
                        componentTitleDiv.click() // Mimic the click operation.
                }else{
                    if(currentlyCollapsed)
                        componentTitleDiv.click()
                }
            }

            if(isCollapseAll)
                collapseAllButton.innerText = i18n.t("inspector.OpenAll")
            else
                collapseAllButton.innerText = i18n.t("inspector.CollapseAll")

            isCollapseAll = !isCollapseAll
        }

        return collapseAllButton
    }

    createMountComponentButton(targetObj){
        let addComponentBtn = document.createElement("button")
        addComponentBtn.innerText = i18n.t("inspector.AddComponent")

        addComponentBtn.onclick = function(){
            let componentForm = formManager.openForm(ComponentListForm)

            let componentNames = huahuoEngine.getAllCompatibleComponents(targetObj)
            componentForm.updateComponentList(componentNames, targetObj)
        }

        return addComponentBtn
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
            try{
                this.contentScrollerDiv.appendChild(contentDiv)
                contentDiv.refresh()
            }catch (e){ // DOMException also can't be caught here ....
                Logger.error("Not sure why, but vanilla hex-input might throw exception, but seems every thing is fine regardless of this exception.")
            }

        }else{
            Logger.info("Selected something")

            let contentDiv = new HHRefreshableDiv()
            contentDiv.style.width="100%"
            this.contentScrollerDiv.appendChild(contentDiv)

            if(targetObj["addComponent"]){
                contentDiv.appendChild(this.createMountComponentButton(targetObj))
            }

            let allComponentTitleDivs:Array<HTMLElement> = new Array<HTMLElement>()
            // Add collapse-all button.
            let properties = propertySheet.getProperties()
            for(let property of properties){
                let divGenerator = GetPropertyDivGenerator(property.type)
                let propertyDesc = divGenerator.generatePropertyDesc(property)

                let propertyDiv = GenerateDiv(divGenerator, propertyDesc)
                contentDiv.appendChild(propertyDiv)

                if(property.type == PropertyType.COMPONENT){
                    allComponentTitleDivs.push(propertyDesc.getTitleDiv())
                }
            }

            if(allComponentTitleDivs.length >= 2){
                contentDiv.insertBefore(this.createOpenCollapseButton(allComponentTitleDivs), contentDiv.firstChild)
            }

            this.shapePropertyDivMapping.set(targetObj, contentDiv)
        }
    }
}

export {Inspector}