import {BasePropertyDesc, BasePropertyDivGenerator, GetPropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";

class GroupPropertyDesc extends BasePropertyDesc{

    private titleTabs:Array<HTMLSpanElement> = new Array<HTMLSpanElement>()
    private contentDivs:Array<HTMLDivElement> = new Array<HTMLDivElement>()

    constructor(property: Property) {
        super(property);

        let groupPropertyDiv = document.createElement("div")

        let titleDivs = document.createElement("div")
        groupPropertyDiv.appendChild(titleDivs)

        let contentDivs = document.createElement("div")
        groupPropertyDiv.appendChild(contentDivs)

        for(let childProperty of property.children){

            let divGenerator = GetPropertyDivGenerator(childProperty.type)
            let propertyDesc = divGenerator.generatePropertyDesc(childProperty)

            titleDivs.appendChild(propertyDesc.getTitleDiv())
            this.titleTabs.push(titleDivs)

            contentDivs.appendChild(propertyDesc.getContentDiv())
            this.contentDivs.push(contentDivs)
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