import {
    BasePropertyDesc,
    BasePropertyDivGenerator,
    GenerateDiv,
    GetPropertyDivGenerator
} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";

class PanelPropertyDesc extends BasePropertyDesc {
    constructor(property: Property) {
        super(property);

        let panelPropertyDiv = document.createElement("div")

        this.contentDiv.appendChild(panelPropertyDiv)
        panelPropertyDiv.className = "accordion-body"
        panelPropertyDiv.style.padding = "0px"

        if (property.config && property.config.children) {
            // TODO: Avoid duplication with Inspector
            for (let childProperty of property.config.children) {
                if (childProperty.hide) { // Some properties are hidden.
                    continue
                }

                // In child property, setter is actually inserter if not explicitly set.
                if(childProperty.inserter && !childProperty.hasOwnProperty("setter")){
                    childProperty.setter = childProperty.inserter
                }
                let divGenerator = GetPropertyDivGenerator(childProperty.type)
                let propertyDesc = divGenerator.generatePropertyDesc(childProperty)

                let propertyDiv = GenerateDiv(divGenerator, propertyDesc)

                panelPropertyDiv.appendChild(propertyDiv)
            }
        }
    }
}

class PanelPropertyDivGenerator extends BasePropertyDivGenerator {
    generatePropertyDesc(property): BasePropertyDesc {
        return new PanelPropertyDesc(property);
    }
}

function makeDivUnselectable(div: HTMLElement) {
    div.style["-webkit-touch-callout"] = "none"
    div.style["-webkit-user-select"] = "none";
    div.style["-khtml-user-select"] = "none";
    div.style["-moz-user-select"] = "none";
    div.style["-ms-user-select"] = "none";
    div.style["user-select"] = "none";
}

let visibleColor = "yellow"
let invisibleColor = "gray"

let currentId = 0;

function getCurrentId(){
    return "component_content_" + currentId++
}

class ComponentPropertyDivGenerator extends BasePropertyDivGenerator {
    generatePropertyDesc(property): BasePropertyDesc {
        let propertyDesc = new PanelPropertyDesc(property);

        let titleDiv = propertyDesc.getTitleDiv()
        titleDiv.classList.add("accordion-header")
        // titleDiv.style.background = visibleColor

        makeDivUnselectable(titleDiv)

        let contentDiv = propertyDesc.getContentDiv()
        contentDiv.className = "accordion-collapse collapse show"
        contentDiv.id = getCurrentId()

        let titleText = titleDiv.innerText
        for(let child of titleDiv.children){
            titleDiv.removeChild(child)
        }
        titleDiv.innerText = ""

        let collapseButton = document.createElement("button")
        collapseButton.className = "accordion-button"
        collapseButton.style.padding = "0px"

        collapseButton.setAttribute("data-bs-toggle", "collapse")
        collapseButton.setAttribute("data-bs-target", "#" + contentDiv.id)
        collapseButton.setAttribute("aria-expanded", "true")
        collapseButton.setAttribute("aria-controls", contentDiv.id)

        collapseButton.innerText = titleText

        titleDiv.appendChild(collapseButton)

        if (property.config && property.config.isActive) {
            let activateButton = document.createElement("input")
            activateButton.className = "btn btn-outline-secondary btn-sm"
            activateButton.type = "button"
            if (property.config.isActive()) {
                activateButton.value = i18n.t("Deactivate")
                activateButton.onclick = this.deActivateComponent(activateButton, property).bind(this)
            } else {
                activateButton.value = i18n.t("Activate")
                activateButton.onclick = this.activateComponent(activateButton, property).bind(this)
            }

            collapseButton.appendChild(activateButton)

            if(property.config && property.config.hasOwnProperty("deleter")){
                let deleteButton = document.createElement("input")
                deleteButton.className = "btn btn-outline-secondary btn-sm"
                deleteButton.type = "button"
                deleteButton.value = i18n.t("DeleteComponent")
                deleteButton.onclick = this.deleteComponent(property).bind(this)

                collapseButton.appendChild(deleteButton)
            }
        }

        return propertyDesc
    }

    activateComponent(activateButton, property) {
        return function (evt: MouseEvent) {
            evt.stopPropagation()
            property.config.enabler()
            activateButton.value = i18n.t("Deactivate")
            activateButton.onclick = this.deActivateComponent(activateButton, property).bind(this)
        }.bind(this)
    }

    deActivateComponent(activateButton, property) {
        return function (evt: MouseEvent) {
            evt.stopPropagation()
            property.config.disabler()
            activateButton.value = i18n.t("Activate")
            activateButton.onclick = this.activateComponent(activateButton, property).bind(this)
        }.bind(this)
    }

    deleteComponent(property){
        return function (evt: MouseEvent){
            evt.stopPropagation()
            property.config.deleter()
        }
    }

    flexDirection(): string {
        return "column"
    }
}

let panelPropertyDivGenerator = new PanelPropertyDivGenerator()
let componentPropertyDivGenerator = new ComponentPropertyDivGenerator()
export {panelPropertyDivGenerator, componentPropertyDivGenerator}