import {HHForm} from "../../Utilities/HHForm";
import {CustomElement} from "hhcommoncomponents";
import {svgShapes} from "../../ShapeDrawers/SVGShapes";
import {CSSUtils} from "../../Utilities/CSSUtils";
import axios from "axios";
import {svgToDataURL} from "../../Utilities/Svgs";
import {huahuoEngine} from "hhenginejs";
import {getMimeTypeFromDataURI} from "hhcommoncomponents";
let md5 = require("js-md5")

@CustomElement({
    selector: "hh-select-icon-form"
})
class SelectIconForm extends HTMLElement implements HHForm {
    selector: string;
    iconsContainerDiv: HTMLDivElement
    closeBtn: HTMLElement

    onIconClicked: Function

    imgSvgMap: Map<HTMLImageElement, string> = new Map() // Map from image element to the svgFile.
    imgShapeNameMap: Map<HTMLImageElement, string> = new Map()

    closeForm() {
        this.style.display = "none"
    }

    onShapeClicked(evt: MouseEvent) {
        evt.preventDefault()
        evt.stopPropagation()

        let _this = this

        let selectedImageEle = evt.target as HTMLImageElement

        let imgURL = this.imgSvgMap.get(selectedImageEle)
        let imgName = this.imgShapeNameMap.get(selectedImageEle)
        axios.get(imgURL).then(response => {
            let data = response["data"]
            if (imgURL.endsWith(".svg")) {
                data = svgToDataURL(data)
            }

            let resourceMD5 = md5(data)

            let mimeType = getMimeTypeFromDataURI(data)
            // Upload the img into the Default resource manager.
            huahuoEngine.LoadBinaryResource(imgName, mimeType, data, data.length)

            if (_this.onIconClicked)
                _this.onIconClicked(resourceMD5)

            _this.closeForm()
        })

    }

    connectedCallback() {
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
        this.closeBtn = this.iconsContainerDiv.querySelector("#closeBtn")
        this.closeBtn.onclick = this.closeForm.bind(this)

        // TODO: Duplicate with IconShapeDrawer.
        // Create Buttons
        for (let shape of svgShapes) {
            let btnImg = document.createElement("img")
            btnImg.src = shape.svg
            btnImg.style.width = "30px"
            btnImg.style.height = "30px"
            btnImg.title = shape.name
            btnImg.addEventListener("click", this.onShapeClicked.bind(this))
            let btn = document.createElement("button")
            btn.style.width = "40px"
            btn.style.height = "40px"
            btn.style.padding = "0px 0px"
            btn.style.border = "1px solid black"
            btn.style.backgroundColor = "lightgray"
            btn.style.margin = "0px"
            btn.appendChild(btnImg)
            form.appendChild(btn)

            this.imgShapeNameMap.set(btnImg, shape.name)
            this.imgSvgMap.set(btnImg, shape.svg)
        }

        this.appendChild(this.iconsContainerDiv)
    }

}

export {SelectIconForm}