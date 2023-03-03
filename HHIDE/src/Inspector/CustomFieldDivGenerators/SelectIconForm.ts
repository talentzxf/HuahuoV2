import {HHForm} from "../../Utilities/HHForm";
import {CustomElement} from "hhcommoncomponents";
import {svgShapes} from "../../ShapeDrawers/SVGShapes";
import {CSSUtils} from "../../Utilities/CSSUtils";

@CustomElement({
    selector: "hh-select-icon-form"
})
class SelectIconForm extends HTMLElement implements HHForm {
    selector: string;
    iconsContainerDiv: HTMLDivElement

    imgSvgMap: Map<HTMLImageElement, string> = new Map() // Map from image element to the svgFile.

    closeForm() {
    }

    onShapeClicked(evt:MouseEvent){
        evt.preventDefault()
        evt.stopPropagation()

    }

    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"
        this.iconsContainerDiv = document.createElement("div")
        this.iconsContainerDiv.innerHTML = CSSUtils.formStyle

        this.iconsContainerDiv.innerHTML += "<form id='iconsContainer'> " +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='closeBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "</form>"

        let form = this.iconsContainerDiv.querySelector("form")

        // TODO: Duplicate with IconShapeDrawer.
        // Create Buttons
        for(let shape of svgShapes){
            let btnImg = document.createElement("img")
            btnImg.src = shape.svg
            btnImg.style.width = "30px"
            btnImg.style.height = "30px"
            btnImg.title = shape.name
            btnImg.addEventListener("click", this.onShapeClicked.bind(this))
            let btn = document.createElement("button")
            btn.style.width = "30px"
            btn.style.height = "30px"
            btn.style.padding = "0px 0px"
            btn.appendChild(btnImg)
            form.appendChild(btn)

            this.imgSvgMap.set(btnImg, shape.svg)
        }

        this.appendChild(this.iconsContainerDiv)
    }

}

export {SelectIconForm}