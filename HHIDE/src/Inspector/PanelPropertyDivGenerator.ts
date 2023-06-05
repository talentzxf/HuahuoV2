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

                this.contentDiv.appendChild(propertyDiv)
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

class ComponentPropertyDivGenerator extends BasePropertyDivGenerator {
    generatePropertyDesc(property): BasePropertyDesc {
        let propertyDesc = new PanelPropertyDesc(property);

        let titleDiv = propertyDesc.getTitleDiv()
        titleDiv.style.background = visibleColor

        makeDivUnselectable(titleDiv)

        let contentDiv = propertyDesc.getContentDiv()

        let contentVisible = false

        titleDiv.setAttribute("isCollapsed", "false")
        titleDiv.addEventListener("click", function () {
            contentVisible = !contentVisible
            if (contentVisible) {
                titleDiv.style.background = invisibleColor
                contentDiv.style.display = "none"

                titleDiv.setAttribute("isCollapsed", "true")
            } else {
                titleDiv.style.background = visibleColor
                contentDiv.style.display = "block"

                titleDiv.setAttribute("isCollapsed", "false")
            }
        })

        if (property.config && property.config.isActive) {
            let activateButtion = document.createElement("input")
            activateButtion.type = "button"
            if (property.config.isActive()) {
                activateButtion.value = i18n.t("Deactivate")
                activateButtion.onclick = this.deActivateComponent(activateButtion, property).bind(this)
            } else {
                activateButtion.value = i18n.t("Activate")
                activateButtion.onclick = this.activateComponent(activateButtion, property).bind(this)
            }

            titleDiv.appendChild(activateButtion)

            if(property.config && property.config.hasOwnProperty("deleter")){
                let deleteButton = document.createElement("input")
                deleteButton.type = "button"
                deleteButton.value = i18n.t("DeleteComponent")
                deleteButton.onclick = this.deleteComponent(property).bind(this)

                titleDiv.appendChild(deleteButton)
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