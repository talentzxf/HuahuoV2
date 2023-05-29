import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {DrawToolBar} from "../UIComponents/DrawToolBar";
import {huahuoEngine, SVGShapeJS} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {svgShapes} from "./SVGShapes";
import axios from "axios";
import {ImageShapeJS} from "hhenginejs";
import {svgToDataURL} from "../Utilities/Svgs";
import {EditorShapeProxy} from "./EditorShapeProxy";

class IconShapeDrawer extends BaseShapeDrawer{
    name = "Shapes"
    imgClass = "fas fa-shapes"

    secondaryToolBar: HTMLDivElement
    imgSvgMap: Map<HTMLImageElement, string> = new Map() // Map from image element to the svgFile.

    selectedImageElement: HTMLImageElement
    tempShape = null


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
        for(let shape of svgShapes){
            let btnImg = document.createElement("img")
            btnImg.src = shape.svg
            btnImg.style.width = "30px"
            btnImg.style.height = "30px"
            btnImg.title = shape.name
            btnImg.addEventListener("click", this.onShapeClicked.bind(this))
            let btn = document.createElement("button")
            btn.appendChild(btnImg)
            this.secondaryToolBar.appendChild(btn)

            this.imgSvgMap.set(btnImg, shape.svg)
        }

        this.secondaryToolBar.style.display = "block"
    }

    onShapeClicked(evt){
        evt.stopPropagation()
        let newlySelectedElement = evt.target as HTMLImageElement

        if(newlySelectedElement == this.selectedImageElement)
            return

        if(this.selectedImageElement != null){
            this.selectedImageElement.style.backgroundColor = "lightgray"
        }

        this.selectedImageElement = newlySelectedElement

        this.selectedImageElement.style.backgroundColor = "green"

        IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEBEGINS, this)
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        if(this.selectedImageElement == null){
            HHToast.warn("Please select a shape from lib first!")
        }else{
            let _this = this
            let imgURL = this.imgSvgMap.get(this.selectedImageElement)
            axios.get(imgURL).then(response=>{
                let data = response["data"]
                if(imgURL.endsWith(".svg")){
                    data = svgToDataURL(data)
                }

                _this.tempShape = EditorShapeProxy.CreateProxy(new ImageShapeJS())
                _this.tempShape.setData(imgURL, data)
                _this.tempShape.createShape()

                _this.tempShape.position = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
                _this.tempShape.store()
            })
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        let _this = this
        huahuoEngine.ExecuteAfterInited(()=>{
            _this.isDrawing = false
            IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)
            _this.addShapeToCurrentLayer(_this.tempShape)
        })
    }
}

export {IconShapeDrawer}