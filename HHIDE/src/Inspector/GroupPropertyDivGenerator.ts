import {BasePropertyDesc, BasePropertyDivGenerator, GetPropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";

class GroupPropertyDesc extends BasePropertyDesc{

    private titleTabs:Array<HTMLSpanElement> = new Array<HTMLSpanElement>()
    private contentDivs:Array<HTMLDivElement> = new Array<HTMLDivElement>()

    constructor(property: Property) {
        super(property);

        let groupPropertyDiv = document.createElement("div")

        groupPropertyDiv.style.borderStyle = "solid"
        groupPropertyDiv.style.borderWidth = "1px"
        groupPropertyDiv.style.borderColor = "blue"

        let titleDivs = document.createElement("div")
        titleDivs.style.display = "flex"
        titleDivs.style.flexDirection = "row"
        groupPropertyDiv.appendChild(titleDivs)

        let contentDivs = document.createElement("div")
        groupPropertyDiv.appendChild(contentDivs)

        let firstProperty = true

        for(let childProperty of property.config.children){

            let divGenerator = GetPropertyDivGenerator(childProperty.type)
            let propertyDesc = divGenerator.generatePropertyDesc(childProperty)

            let titleDiv = propertyDesc.getTitleDiv()
            if(firstProperty)
                titleDiv.style.background = "darkgray"
            else
                titleDiv.style.background = "lightgray"

            titleDivs.appendChild(titleDiv)
            this.titleTabs.push(titleDiv)

            let contentDiv = propertyDesc.getContentDiv()

            if(firstProperty)
                contentDiv.style.display = "block"
            else
                contentDiv.style.display = "none"

            contentDivs.appendChild(contentDiv)
            this.contentDivs.push(contentDiv)

            let _this = this
            titleDiv.addEventListener("mousedown", function(){
                for(let titleTab of _this.titleTabs){
                    titleTab.style.background = "lightgray"
                }

                titleDiv.style.background = "darkgray"

                for(let candidateDiv of _this.contentDivs){
                    candidateDiv.style.display = "none"
                }

                contentDiv.style.display = "block"
            })

            firstProperty = false
        }

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