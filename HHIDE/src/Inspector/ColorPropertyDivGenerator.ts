import {BasePropertyDivGenerator, BasePropertyDesc} from "./BasePropertyDivGenerator";
import {Property} from "hhcommoncomponents"
import {paper} from "hhenginejs"

import "vanilla-colorful"
import "vanilla-colorful/rgba-string-color-picker.js";
import "vanilla-colorful/hex-input.js"
import {RgbaStringColorPicker} from "vanilla-colorful/rgba-string-color-picker";
import {HexInput} from "vanilla-colorful/hex-input";

function colorToHex(color:paper.Color){
    let alpha = Math.round(color.alpha * 255).toString(16)
    return color.toCSS(true) + alpha
}

function hexToRgbString(hexStr){
    let strippedStr = hexStr.replace(/[^0-9a-f]/g,"")
    let components = strippedStr.match(/.{1,2}/g)

    if(components.length == 3){
        return "rgb(" +
            parseInt(components[0], 16).toString(10) + "," +
            parseInt(components[1], 16).toString(10) + "," +
            parseInt(components[2], 16).toString(10)  + ")"
    }

    if(components.length == 4){
        return "rgba(" +
            parseInt(components[0], 16).toString(10) + "," +
            parseInt(components[1], 16).toString(10) + "," +
            parseInt(components[2], 16).toString(10) + "," +
            parseInt(components[3], 16).toString(10) + ")"
    }
}

function rgbStringToHex(colorStr){
    let strippedColorStr = colorStr.replace(/[^\d,]/g,'')
    let components = strippedColorStr.split(",")
    let resultStr = ""
    for(let component of components){
        let componentValue = parseInt(component)
        resultStr += ("00" + componentValue.toString(16)).slice(-2)
    }

    return resultStr
}

class ColorPropertyDiv extends BasePropertyDesc{
    colorPicker: RgbaStringColorPicker
    colorInput: HexInput
    showMoreButton: HTMLButtonElement
    isExpanded: boolean = true
    silentColorChangeEvent:boolean = false

    getButtonText(){
        if(this.isExpanded)
            return "Hide"
        return "Show"
    }

    onValueChanged(val:paper.Color){
        this.silentColorChangeEvent = true
        this.colorPicker.color = val.toCSS()
        this.silentColorChangeEvent = false
    }

    constructor(property: Property) {
        super(property)

        this.colorPicker = document.createElement("rgba-string-color-picker")

        let currentColor = property.getter()
        this.colorPicker.color = currentColor.toCSS()

        this.colorPicker.addEventListener("color-changed", e=>{

            if(this.silentColorChangeEvent)
                return
            
            let newColor = new paper.Color(e.detail.value)
            this.setter(newColor)

            if(e.detail.value.startsWith("rgb") && this.colorInput.color != e.detail.value){
                this.colorInput.color = rgbStringToHex(this.colorPicker.color)
            }
        })

        this.contentDiv.appendChild(this.colorPicker)

        this.createTitleDiv()
        this.colorInput.color = colorToHex(currentColor)
    }

    showMoreButtonClicked(){
        if(this.isExpanded){ // Currently expanded, collapse
            this.colorPicker.style.display = "none"
        } else {
            this.colorPicker.style.display = "flex"
        }

        this.isExpanded = !this.isExpanded
        this.showMoreButton.innerText = this.getButtonText()
    }

    createTitleDiv(){
        this.titleDiv.style.display = "flex"
        this.titleDiv.style.flexDirection = "row"
        this.colorInput = document.createElement("hex-input")
        this.colorInput.style.width = "80px"
        this.titleDiv.appendChild(this.colorInput)

        this.colorInput.addEventListener("color-changed", e=>{
            let rgbString = hexToRgbString(e.detail.value)
            if(rgbString)
                this.colorPicker.color = rgbString
        })

        let _this = this
        setTimeout( ()=>{
            let input = _this.colorInput.shadowRoot.querySelector("input")
            if(input)
                input.style.width = "60px"
        },0)

        this.showMoreButton = document.createElement("button")
        this.showMoreButton.innerText = this.getButtonText()
        this.showMoreButton.onclick = this.showMoreButtonClicked.bind(this)

        this.titleDiv.appendChild(this.showMoreButton)
    }
}

class ColorPropertyDivGenerator extends BasePropertyDivGenerator{
    generatePropertyDesc(property): BasePropertyDesc {
        return new ColorPropertyDiv(property)
    }

    flexDirection(): string {
        return "column"
    }
}

let colorPropertyDivGenerator = new ColorPropertyDivGenerator()

export {colorPropertyDivGenerator}
