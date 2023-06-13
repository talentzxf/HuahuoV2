import {CustomElement, Logger, PropertySheet} from "hhcommoncomponents"
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {GenerateDiv, GetPropertyDivGenerator} from "./BasePropertyDivGenerator"
import "./PropertyTypes"
import {findParentSideBar, findParentPanel} from "hhpanel";
import {PropertyType} from "hhcommoncomponents";
import {formManager} from "../Utilities/FormManager";
import {ComponentListForm} from "../UIComponents/ComponentListForm";
import {huahuoEngine} from "hhenginejs";
import {HHRefreshableDiv} from "./InputComponents/HHRefreshableDiv";
import {HHToast} from "hhcommoncomponents";
import {saveAs} from "file-saver";

@CustomElement({
    selector: "hh-inspector"
})
class Inspector extends HTMLElement{
    contentScrollerDiv:HTMLDivElement;
    shapePropertyDivMapping: Map<any, HHRefreshableDiv> = new Map()

    selectedObj: any = null

    connectedCallback() {
        let parentPanel = findParentPanel(this)
        let parentHeight = parentPanel?parentPanel.clientHeight: 500;

        this.contentScrollerDiv = document.createElement("div")
        this.contentScrollerDiv .style.width = "100%"
        this.contentScrollerDiv .style.height = parentHeight + "px"
        this.contentScrollerDiv .style.overflowY = "auto"
        this.contentScrollerDiv.style.overflowX = "auto"
        this.contentScrollerDiv.style.resize = "both"
        this.appendChild(this.contentScrollerDiv )

        IDEEventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))
        IDEEventBus.getInstance().on(EventNames.UNSELECTOBJECTS, this.unselectObjects.bind(this))

        IDEEventBus.getInstance().on(EventNames.COMPONENTCHANGED, this.componentChanged.bind(this))

        IDEEventBus.getInstance().on(EventNames.CELLCLICKED, this.timelineCellClicked.bind(this))

        IDEEventBus.getInstance().on(EventNames.OBJECTDELETED, this.objectDeleted.bind(this))

        findParentSideBar(this).hide()
    }

    timelineCellClicked(){
        let refreshableDiv = this.querySelector("hh-refreshable-div") as HHRefreshableDiv
        if(refreshableDiv){
            refreshableDiv.refresh()
        }
    }

    unselectObjects(){
        findParentSideBar(this).hide()
    }

    componentChanged(targetObj: any){
        if(this.shapePropertyDivMapping.has(targetObj)){
            this.shapePropertyDivMapping.delete(targetObj)
        }

        this.onItemSelected(targetObj.getPropertySheet(), targetObj)
    }

    createOpenCollapseButton(allComponentTitleDivs: Array<HTMLElement>){

        let collapseAllButton = document.createElement("button")
        collapseAllButton.className = "btn btn-primary btn-sm"
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
        addComponentBtn.className = "btn btn-primary btn-sm"
        addComponentBtn.innerText = i18n.t("inspector.AddComponent")

        addComponentBtn.onclick = function(){
            let componentForm = formManager.openForm(ComponentListForm)

            let componentNames = huahuoEngine.getAllCompatibleComponents(targetObj)
            componentForm.updateComponentList(componentNames, targetObj)
        }

        return addComponentBtn
    }

    createSaveAsKeyFrameButton(targetObj){
        let saveAsKeyFrameBtn = document.createElement("button")
        saveAsKeyFrameBtn.className = "btn btn-primary btn-sm"
        saveAsKeyFrameBtn.innerText = i18n.t("inspector.SaveAsKeyFrame")

        saveAsKeyFrameBtn.onclick = function(){
            targetObj.saveAsKeyFrame()

            HHToast.info(i18n.t("toast.keyframeSaved"))
        }

        return saveAsKeyFrameBtn
    }

    objectDeleted(targetObj){
        if(this.selectedObj == targetObj){
            findParentSideBar(this).hide()

            this.shapePropertyDivMapping.delete(targetObj)
        }
    }

    onItemSelected(propertySheet: PropertySheet, targetObj: any){
        try {
            this.selectedObj = targetObj

            findParentSideBar(this).show()
            let parentContainer = findParentSideBar(this)
            let titleBarHeight = parentContainer.querySelector(".title_tabs").offsetHeight
            let parentHeight = parentContainer.clientHeight - titleBarHeight;
            if (parentHeight == 0) {
                parentHeight = 500 // Restore to default height.
            }

            this.contentScrollerDiv.style.height = (parentHeight) + "px"

            while (this.contentScrollerDiv.firstChild) {
                this.contentScrollerDiv.removeChild(this.contentScrollerDiv.firstChild)
            }

            if (this.shapePropertyDivMapping.has(targetObj)) {
                let contentDiv = this.shapePropertyDivMapping.get(targetObj)
                try {
                    this.contentScrollerDiv.appendChild(contentDiv)
                    contentDiv.refresh()
                } catch (e) { // DOMException also can't be caught here ....
                    Logger.error("Not sure why, but vanilla hex-input might throw exception, but seems every thing is fine regardless of this exception.")
                }

            } else {
                Logger.info("Selected something")

                let contentDiv = new HHRefreshableDiv()
                contentDiv.style.width = "100%"
                this.contentScrollerDiv.appendChild(contentDiv)

                let basicFunctionsGroup = document.createElement("div")
                basicFunctionsGroup.className = "btn-group"
                contentDiv.appendChild(basicFunctionsGroup)

                if (targetObj["addComponent"]) {
                    basicFunctionsGroup.appendChild(this.createMountComponentButton(targetObj))
                }

                if (targetObj["saveAsKeyFrame"]) {
                    basicFunctionsGroup.appendChild(this.createSaveAsKeyFrameButton(targetObj))
                }

                let allComponentTitleDivs: Array<HTMLElement> = new Array<HTMLElement>()
                // Add collapse-all button.
                let properties = propertySheet.getProperties()
                for (let property of properties) {
                    if (property.hide)
                        continue;

                    let divGenerator = GetPropertyDivGenerator(property.type)
                    let propertyDesc = divGenerator.generatePropertyDesc(property)

                    let propertyDiv = GenerateDiv(divGenerator, propertyDesc)
                    contentDiv.appendChild(propertyDiv)

                    if (property.type == PropertyType.COMPONENT) {
                        allComponentTitleDivs.push(propertyDesc.getTitleDiv())
                    }
                }

                if (allComponentTitleDivs.length >= 2) {
                    basicFunctionsGroup.insertBefore(this.createOpenCollapseButton(allComponentTitleDivs), basicFunctionsGroup.firstChild)
                }

                this.shapePropertyDivMapping.set(targetObj, contentDiv)
            }
        }catch (e){
            console.log("Error happened while trying to show inspector")
            console.log(e)
        }
    }
}

export {Inspector}