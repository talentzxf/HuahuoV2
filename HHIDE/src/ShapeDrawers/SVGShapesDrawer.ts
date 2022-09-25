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
            svg: "foolball.svg"
        }
    ]
    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);

        let drawToolBar: DrawToolBar = document.querySelector("hh-draw-toolbar")

        this.secondaryToolBar = drawToolBar.getSecondaryToolBar()

        this.secondaryToolBar.innerHTML = null;

        // Create Buttons
        for(let shape of this.shapes){
            let btnImg = document.createElement("img")
            btnImg.src = shape.svg
            this.secondaryToolBar.appendChild(btnImg)
        }

        this.secondaryToolBar.style.display = "block"
    }
}

export {SVGShapesDrawer}