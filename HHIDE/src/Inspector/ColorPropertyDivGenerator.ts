import {BasePropertyDivGenerator} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"
import {paper} from "hhenginejs"

import "vanilla-colorful"
import "vanilla-colorful/rgba-string-color-picker.js";
import {RgbaStringColorPicker} from "vanilla-colorful/rgba-string-color-picker";

class ColorPropertyDiv{
    contentDiv: HTMLDivElement
    colorPicker: RgbaStringColorPicker
    setter: Function

    onValueChanged(val:paper.Color){

    }

    constructor(property: Property) {
        property.registerValueChangeFunc(this.onValueChanged.bind(this))

        this.contentDiv = document.createElement("div")

        this.colorPicker = document.createElement("rgba-string-color-picker")

        let currentColor = property.getter()
        this.setter = property.setter

        this.colorPicker.color = currentColor.toCSS()

        this.colorPicker.addEventListener("color-changed", e=>{
            let newColor = new paper.Color(e.detail.value)
            this.setter(newColor)
        })

        this.contentDiv.appendChild(this.colorPicker)
    }
}

class ColorPropertyDivGenerator extends BasePropertyDivGenerator{
    generateDiv(property): HTMLElement {
        return new ColorPropertyDiv(property).contentDiv
    }

    flexDirection(): string {
        return "column"
    }
}

let colorPropertyDivGenerator = new ColorPropertyDivGenerator()

export {colorPropertyDivGenerator}
