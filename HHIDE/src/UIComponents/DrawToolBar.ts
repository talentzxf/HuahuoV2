import {shapes} from "../ShapeDrawers/Shapes";
import {CustomElement} from "hhcommoncomponents"
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";

@CustomElement({
    selector: "hh-draw-toolbar"
})
class DrawToolBar extends HTMLElement{
    connectedCallback(){

        for(let shape of shapes){
            this.createButton(shape)
        }

    }

    createButton(shape: BaseShapeDrawer){
        let img = document.createElement("img")
        img.className = shape.imgClass
        img.onclick = shape.onClicked.bind(shape)

        let button = document.createElement('button')
        button.append(img)
        this.append(button)
    }

    onClick(){

    }
}



export {DrawToolBar}