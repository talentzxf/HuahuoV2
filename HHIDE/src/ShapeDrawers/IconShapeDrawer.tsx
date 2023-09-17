import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {huahuoEngine, ImageShapeJS} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {svgShapes} from "./SVGShapes";
import axios from "axios";
import {SVGFiles, svgToDataURL} from "../Utilities/Svgs";
import {EditorShapeProxy} from "./EditorShapeProxy";
import {fileLoader} from "../SceneView/FileLoader";
import * as React from "react";
import {ReactNode} from "react";

class IconShapeDrawer extends BaseShapeDrawer {
    name = "Shapes"
    imgClass = "fas fa-shapes"

    secondaryToolBar: HTMLDivElement
    shapeNameSvgMap: Map<string, string> = new Map() // Map from shape name to SVGFile content.

    selectedIconName: string = null
    tempShape = null


    set isSelected(val: boolean) {
        super.isSelected = val
        if (this.secondaryToolBar) {
            this.secondaryToolBar.style.display = "none"
        }
    }

    canvas: HTMLCanvasElement

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        if (canvas)
            this.canvas = canvas
    }

    getSecondaryDrawToolBar(): ReactNode {
        let iconButtons = []
        for (let shape of svgShapes) {
            this.shapeNameSvgMap.set(shape.name, shape.svg)
            let button = (
                <button key={shape.name}
                        className="m-2 w-6 h-6 hover:w-8 hover:m-1 hover:h-8 transition-all ease-in-out"
                        onClick={this.onShapeClicked.bind(this)} data-icon-name={shape.name}>
                    <img src={shape.svg} alt={shape.name} title={shape.name}/>
                </button>
            )

            iconButtons.push(button)
        }

        return (
            <>
                {iconButtons}
            </>
        )
    }

    onShapeClicked(evt) {
        evt.stopPropagation()

        this.selectedIconName = evt.currentTarget.dataset.iconName
        IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEBEGINS, this)

        if (this.canvas) {
            let shapeSvg = this.shapeNameSvgMap.get(this.selectedIconName)
            this.canvas.style.cursor = "url(\"" + shapeSvg + "\") 0 32, pointer"
        }
    }

    onMouseDown(evt: MouseEvent) {
        super.onMouseDown(evt);

        if (this.selectedIconName == null) {
            HHToast.warn("Please select a shape from lib first!")
        } else {
            let _this = this
            let imgURL = this.shapeNameSvgMap.get(this.selectedIconName)
            axios.get(imgURL).then(response => {
                let data = response["data"]
                if (imgURL.endsWith(".svg")) {
                    data = svgToDataURL(data)
                }

                let resourceMD5 = fileLoader.loadBinaryDataIntoStore(imgURL, data)

                _this.tempShape = EditorShapeProxy.CreateProxy(new ImageShapeJS())
                _this.tempShape.setResourceByMD5(resourceMD5)
                _this.tempShape.createShape()

                _this.tempShape.position = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
                _this.tempShape.store()
            })
        }
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        let _this = this
        huahuoEngine.ExecuteAfterInited(() => {
            _this.isDrawing = false
            IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEENDS, _this)
            _this.addShapeToCurrentLayer(_this.tempShape)
        })
    }
}

export {IconShapeDrawer}