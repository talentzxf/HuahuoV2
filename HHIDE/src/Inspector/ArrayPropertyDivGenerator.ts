import {
    BasePropertyDesc,
    BasePropertyDivGenerator,
    GenerateDiv,
    GetPropertyDivGenerator
} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents";
import {PropertyType} from "hhcommoncomponents";

class ArrayPropertyDesc extends BasePropertyDesc{
    arrayEntryDivs: HTMLDivElement
    elementType: PropertyType

    constructor(property: Property) {
        super(property);

        let propertyListDiv = document.createElement("div")
        this.contentDiv.appendChild(propertyListDiv)

        let addButton = document.createElement("button")
        addButton.innerText = "+"
        this.getTitleDiv().appendChild(addButton)
        addButton.addEventListener("click", this.addEntry.bind(this))

        this.arrayEntryDivs = document.createElement("div")
        this.contentDiv.appendChild(this.arrayEntryDivs)

        this.elementType = property.elementType

        let currentArray = property.getter()
        for(let entry of currentArray){
            let desc = this.addEntry()
            if(desc)
                desc["onValueChanged"](entry)
        }
    }

    addEntry(){
        let propertyDivGenerator = GetPropertyDivGenerator(this.elementType)
        let propertyDesc = propertyDivGenerator.generatePropertyDesc(this.property)

        let generatedDiv = GenerateDiv(propertyDivGenerator, propertyDesc)

        this.arrayEntryDivs.appendChild(generatedDiv)

        return propertyDesc

    }

    onValueChanged(val) {
    }
}


class ArrayPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new ArrayPropertyDesc(property)
    }

    flexDirection(): string {
        return "column"
    }
}

let arrayPropertyDivGenerator = new ArrayPropertyDivGenerator()

export {arrayPropertyDivGenerator, ArrayPropertyDesc}