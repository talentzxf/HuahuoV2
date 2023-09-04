import {CustomElement, HHToast, Logger, PropertySheet, PropertyType} from "hhcommoncomponents"
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {GenerateDiv, GetPropertyDivGenerator} from "./BasePropertyDivGenerator"
import "./PropertyTypes"
import {findParentPanel, findParentSideBar} from "hhpanel";
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

        let _this = this
        collapseAllButton.onclick = function(){
            let allAccordionButtons = _this.querySelectorAll(".accordion-button")

            for(let button of allAccordionButtons){
                let targetId = button.getAttribute("data-bs-target")
                let target = _this.querySelector(targetId)

                if(target.classList.contains("show") && isCollapseAll){
                    (button as HTMLButtonElement).click()
                }else if(!target.classList.contains("show") && !isCollapseAll){
                    (button as HTMLButtonElement).click()
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

    createShowCenterSelectorButton(targetObj){
        let showCenterSelectorButton = document.createElement("button")
        showCenterSelectorButton.className = 'btn btn-primary btn-sm'
        showCenterSelectorButton.innerText = i18n.t("inspector.ShowCenterSelector")

        showCenterSelectorButton.onclick = function(){
            if(!targetObj.isShowCenterSelector()){
                showCenterSelectorButton.innerText = i18n.t("inspector.HideCenterSelector")

                targetObj.showCenterSelector()
            }else{
                showCenterSelectorButton.innerText = i18n.t("inspector.ShowCenterSelector")

                targetObj.hideCenterSelector()
            }
        }

        return showCenterSelectorButton
    }

    createLockButton(targetObj){
        let lockObjectButton = document.createElement("button")
        lockObjectButton.className = "btn btn-primary btn-sm"
        lockObjectButton.innerText = i18n.t("inspector.LockObject")
        lockObjectButton.onclick = function(){
            if(!targetObj.isLocked()){
                lockObjectButton.innerText = i18n.t("inspector.UnlockObject")
            }else{
                lockObjectButton.innerText = i18n.t("inspector.LockObject")
            }

            targetObj.setIsLocked( !targetObj.isLocked())
        }

        return lockObjectButton
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

                if(targetObj["isLocked"] && targetObj["isLocked"] instanceof Function){
                    basicFunctionsGroup.appendChild(this.createLockButton(targetObj))
                }

                if(targetObj["isShowCenterSelector"] && targetObj["isShowCenterSelector"] instanceof Function){
                    basicFunctionsGroup.appendChild(this.createShowCenterSelectorButton(targetObj))
                }

                let componentDivs = document.createElement("div")
                componentDivs.className = "accordion"
                contentDiv.appendChild(componentDivs)

                let allComponentTitleDivs: Array<HTMLElement> = new Array<HTMLElement>()
                // Add collapse-all button.
                let properties = propertySheet.getProperties()
                for (let property of properties) {
                    if (property.hide)
                        continue;

                    let divGenerator = GetPropertyDivGenerator(property.type)
                    let propertyDesc = divGenerator.generatePropertyDesc(property)

                    let propertyDiv = GenerateDiv(divGenerator, propertyDesc)
                    propertyDiv.className = "accordion-item"
                    componentDivs.appendChild(propertyDiv)

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