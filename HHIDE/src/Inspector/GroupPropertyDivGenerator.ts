import {BasePropertyDesc, BasePropertyDivGenerator, GetPropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";

class GroupPropertyDesc extends BasePropertyDesc{

    createTabs(parentDiv: HTMLDivElement, property: Property){
        let titleTabElements = []
        let contentDivElements = []

        let titleDivs = document.createElement("div")
        titleDivs.style.display = "flex"
        titleDivs.style.flexDirection = "row"
        parentDiv.appendChild(titleDivs)

        let contentDivs = document.createElement("div")
        parentDiv.appendChild(contentDivs)

        let firstProperty = true

        for(let childProperty of property.config.children){
            if(childProperty.hide)
                continue

            let divGenerator = GetPropertyDivGenerator(childProperty.type)
            let propertyDesc = divGenerator.generatePropertyDesc(childProperty)

            let titleDiv = propertyDesc.getTitleDiv()
            if(firstProperty)
                titleDiv.style.background = "darkgray"
            else
                titleDiv.style.background = "lightgray"

            titleDivs.appendChild(titleDiv)
            titleTabElements.push(titleDiv)

            let contentDiv = propertyDesc.getContentDiv()

            if(firstProperty)
                contentDiv.style.display = "block"
            else
                contentDiv.style.display = "none"

            contentDivs.appendChild(contentDiv)
            contentDivElements.push(contentDiv)

            let _this = this
            titleDiv.addEventListener("mousedown", function(){
                for(let titleTab of titleTabElements){
                    titleTab.style.background = "lightgray"
                }

                titleDiv.style.background = "darkgray"

                for(let candidateDiv of contentDivElements){
                    candidateDiv.style.display = "none"
                }

                contentDiv.style.display = "block"
            })

            firstProperty = false
        }
    }

    constructor(property: Property) {
        super(property);

        let groupPropertyDiv = document.createElement("div")

        groupPropertyDiv.style.borderStyle = "solid"
        groupPropertyDiv.style.borderWidth = "1px"
        groupPropertyDiv.style.borderColor = "blue"

        this.createTabs(groupPropertyDiv, property)
        this.contentDiv.appendChild(groupPropertyDiv)
    }

    onValueChanged(val) {
    }
}

class GroupPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new GroupPropertyDesc(property);
    }
}

let groupPropertyDivGenerator = new GroupPropertyDivGenerator()
export {groupPropertyDivGenerator}