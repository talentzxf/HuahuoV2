import {
    BasePropertyDesc,
    BasePropertyDivGenerator,
    GenerateDiv,
    GetPropertyDivGenerator
} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";

class PanelPropertyDesc extends BasePropertyDesc{
    constructor(property: Property) {
        super(property);

        let panelPropertyDiv = document.createElement("div")

        this.contentDiv.appendChild(panelPropertyDiv)

        // TODO: Avoid duplication with Inspector
        for(let childProperty of property.config.children){
            let divGenerator = GetPropertyDivGenerator(childProperty.type)
            let propertyDesc = divGenerator.generatePropertyDesc(childProperty)

            let propertyDiv = GenerateDiv(divGenerator, propertyDesc)

            this.contentDiv.appendChild(propertyDiv)
        }
    }
    onValueChanged(val) {
    }
}

class PanelPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new PanelPropertyDesc(property);
    }
}

function makeDivUnselectable(div: HTMLElement){
    div.style["-webkit-touch-callout"] = "none"
    div.style["-webkit-user-select"] = "none";
    div.style["-khtml-user-select"] = "none";
    div.style["-moz-user-select"] = "none";
    div.style["-ms-user-select"] = "none";
    div.style["user-select"] = "none";
}

let visibleColor = "yellow"
let invisibleColor = "gray"
class ComponentPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        let propertyDesc = new PanelPropertyDesc(property);

        let titleDiv = propertyDesc.getTitleDiv()
        titleDiv.style.background = visibleColor

        makeDivUnselectable(titleDiv)

        let contentDiv = propertyDesc.getContentDiv()

        let contentVisible = false

        titleDiv.setAttribute("isCollapsed", "false")
        titleDiv.addEventListener("click", function(){
            contentVisible = !contentVisible
            if(contentVisible){
                titleDiv.style.background = invisibleColor
                contentDiv.style.display = "none"

                titleDiv.setAttribute("isCollapsed", "true")
            }
            else{
                titleDiv.style.background = visibleColor
                contentDiv.style.display = "block"

                titleDiv.setAttribute("isCollapsed", "false")
            }
        })

        return propertyDesc
    }

    flexDirection(): string {
        return "column"
    }
}

let panelPropertyDivGenerator = new PanelPropertyDivGenerator()
let componentPropertyDivGenerator = new ComponentPropertyDivGenerator()
export {panelPropertyDivGenerator, componentPropertyDivGenerator}