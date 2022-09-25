import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {DrawToolBar} from "../UIComponents/DrawToolBar";
import {SVGFiles} from "../Utilities/Svgs";

class SVGShapesDrawer extends BaseShapeDrawer{
    name = "Shapes"
    imgClass = "fas fa-shapes"

    secondaryToolBar: HTMLDivElement

    shapes = [
        {
            name: "football",
            svg: "svgs/football.svg"
        },
        {
            name: "bus",
            svg: "svgs/bus.svg"
        }
    ]

    set isSelected(val:boolean){
        super.isSelected = val
        if(this.secondaryToolBar){
            this.secondaryToolBar.style.display = "none"
        }
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);

        let drawToolBar: DrawToolBar = document.querySelector("hh-draw-toolbar")

        this.secondaryToolBar = drawToolBar.getSecondaryToolBar()

        this.secondaryToolBar.innerHTML = null;

        // Create Buttons
        for(let shape of this.shapes){
            let btnImg = document.createElement("img")
            btnImg.src = shape.svg
            btnImg.style.width = "30px"
            btnImg.style.height = "30px"
            this.secondaryToolBar.appendChild(btnImg)
        }

        this.secondaryToolBar.style.display = "block"
    }
}

export {SVGShapesDrawer}