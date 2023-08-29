import "vanilla-colorful"
import "vanilla-colorful/rgba-string-color-picker.js";

import {RgbaStringColorPicker} from "vanilla-colorful/rgba-string-color-picker";
import {CustomElement} from "hhcommoncomponents";
import {SetFieldValueCommand} from "../../RedoUndo/SetFieldValueCommand";
import {undoManager} from "../../RedoUndo/UndoManager";

function colorToHex(color: paper.Color) {
    let alpha = Math.round(color.alpha * 255).toString(16)
    return color.toCSS(true) + alpha
}

function rgbStringToHex(colorStr) {
    let resultStr = ""
    let strippedColorStr = colorStr.replace(/[^\d,.]/g, '')
    let components = strippedColorStr.split(",")
    for (let componentIdx = 0; componentIdx < 3; componentIdx++) {
        let componentValue = parseInt(components[componentIdx])
        resultStr += ("00" + componentValue.toString(16)).slice(-2)
    }
    if (colorStr.startsWith("rgba")) {
        let alphaValue = Math.floor(parseFloat(components[3]) * 255).toString(16)
        return resultStr + ("00" + alphaValue).slice(-2)
    } else if (colorStr.startsWith("rgb")) {
        resultStr += "ff" // Append "ff" as the color is totally opaque
        return resultStr
    } else {
        throw "Unknown color format!" + colorStr
    }
}

function hexToRgbString(hexStr) {
    let strippedStr = hexStr.replace(/[^0-9a-f]/g, "")
    let components = strippedStr.match(/.{1,2}/g)

    if (components.length == 3) {
        return "rgb(" +
            parseInt(components[0], 16).toString(10) + "," +
            parseInt(components[1], 16).toString(10) + "," +
            parseInt(components[2], 16).toString(10) + ")"
    }

    if (components.length == 4) {
        return "rgba(" +
            parseInt(components[0], 16).toString(10) + "," +
            parseInt(components[1], 16).toString(10) + "," +
            parseInt(components[2], 16).toString(10) + "," +
            parseInt(components[3], 16)/255.0 + ")"
    }
}

@CustomElement({
    selector: "hh-color-input"
})
class HHColorInput extends HTMLElement implements RefreshableComponent {
    colorPicker: RgbaStringColorPicker
    colorInput: HTMLInputElement
    colorIndicator: HTMLDivElement
    showMoreButton: HTMLButtonElement
    isExpanded: boolean = false
    silentColorChangeEvent: boolean = false

    setter: Function
    getter: Function

    /*
    Title Div need to be passed in. Can't mount in the content part
     */
    constructor(setter, getter, titleDiv) {
        super();

        this.setter = setter
        this.getter = getter

        this.colorPicker = document.createElement("rgba-string-color-picker")

        this.colorPicker.addEventListener("color-changed", e => {

            if (this.silentColorChangeEvent)
                return

            let oldColor = this.getter()
            let newColor = new paper.Color(e.detail.value)

            let setColorCommand = new SetFieldValueCommand<paper.Color>(this.setter, oldColor, newColor)
            setColorCommand.DoCommand()

            undoManager.PushCommand(setColorCommand)

            if (e.detail.value.startsWith("rgb") && this.colorInput.value != e.detail.value) {
                this.colorInput.value = rgbStringToHex(this.colorPicker.color)
                this.colorIndicator.style.backgroundColor = this.colorPicker.color
            }
        })

        this.appendChild(this.colorPicker)

        this.createTitleDiv(titleDiv)

        this.refresh()
    }

    getButtonText() {
        if (this.isExpanded)
            return "Hide"
        return "Show"
    }

    set value(val: any) {
        this.silentColorChangeEvent = true
        this.colorPicker.color = val.toCSS()
        this.colorInput.value = colorToHex(val)
        this.colorIndicator.style.backgroundColor = val.toCSS()
        this.silentColorChangeEvent = false
    }

    createTitleDiv(titleDiv) {
        titleDiv.style.display = "flex"
        titleDiv.style.flexDirection = "row"
        this.colorInput = document.createElement("input")
        this.colorInput.className = "form-control form-control-sm col-md"
        titleDiv.appendChild(this.colorInput)

        this.colorIndicator = document.createElement("div")
        this.colorIndicator.className = "col-md"
        this.colorIndicator.style.border = "1px solid black"
        this.colorIndicator.style.height = this.colorInput.size + "px"

        titleDiv.appendChild(this.colorIndicator)

        let _this = this
        this.colorInput.addEventListener("input", e => {
            if (_this.silentColorChangeEvent)
                return

            let rgbString = hexToRgbString(this.colorInput.value)
            if (rgbString){
                this.colorPicker.color = rgbString
                this.colorIndicator.style.background = rgbString
            }
        })

        setTimeout(() => {
            if (_this.colorInput.shadowRoot) {
                let input = _this.colorInput.shadowRoot.querySelector("input")
                if (input)
                    input.style.width = "60px"
            }
        }, 0)

        this.showMoreButton = document.createElement("button")
        this.showMoreButton.className = "btn btn-outline-secondary col-md"
        this.showMoreButton.innerText = this.getButtonText()
        this.showMoreButton.onclick = this.showMoreButtonClicked.bind(this)

        if (!this.isExpanded) {
            this.colorPicker.style.display = "none"
        }

        titleDiv.appendChild(this.showMoreButton)
    }

    hideColorSelector() {
        this.colorPicker.style.display = "none"
        this.isExpanded = false
    }

    showMoreButtonClicked() {
        if (this.isExpanded) { // Currently expanded, collapse
            this.colorPicker.style.display = "none"
        } else {
            this.colorPicker.style.display = "flex"
        }

        this.isExpanded = !this.isExpanded
        this.showMoreButton.innerText = this.getButtonText()
    }

    refresh() {
        this.silentColorChangeEvent = true
        let currentColor = this.getter()
        this.colorPicker.color = currentColor.toCSS()
        this.colorInput.value = colorToHex(currentColor)
        this.silentColorChangeEvent = false
        this.colorIndicator.style.backgroundColor = currentColor.toCSS()
    }
}

export {HHColorInput}