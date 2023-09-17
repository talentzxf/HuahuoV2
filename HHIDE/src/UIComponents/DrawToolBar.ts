import {shapes} from "../ShapeDrawers/Shapes";
import {CustomElement, Logger} from "hhcommoncomponents"
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {createRoot} from "react-dom/client"
import {DrawToolBarX} from "./DrawToolBarX";
import * as React from "react"

@CustomElement({
    selector: "hh-draw-toolbar"
})
class DrawToolBar extends HTMLElement {
    private shapeButtonMap: Map<BaseShapeDrawer, HTMLButtonElement> = new Map
    private currentActiveDrawer: BaseShapeDrawer = null

    private buttonContainer: HTMLDivElement
    private secondaryDrawToolBar: HTMLDivElement

    setButtonBackgroundColor(button: HTMLButtonElement, isSelected: boolean) {
        let bgColor = "white"
        if (isSelected)
            bgColor = "#42b983";

        button.style.backgroundColor = bgColor
    }

    initializeTools() {
        this.buttonContainer = document.createElement("div")
        this.buttonContainer.className = "btn-group btn-group-sm"
        this.appendChild(this.buttonContainer)

        this.secondaryDrawToolBar = document.createElement("div")
        this.secondaryDrawToolBar.className = "btn-group btn-group-sm"
        this.appendChild(this.secondaryDrawToolBar)

        for (let shape of shapes) {
            this.createButton(shape)
        }

        IDEEventBus.getInstance().on(EventNames.DRAWSHAPEBEGINS, this.onDrawShapeBegins.bind(this))
        IDEEventBus.getInstance().on(EventNames.DRAWSHAPEENDS, this.onEndOfDrawingShape.bind(this))
    }

    getSecondaryToolBar(): HTMLDivElement {
        return this.secondaryDrawToolBar
    }

    connectedCallback() {
        // (window as any).i18n.ExecuteAfterInited(this.initializeTools.bind(this))

        // Attach React component
        let reactRoot = createRoot(this)
        let toolBarX = React.createElement(DrawToolBarX)
        reactRoot.render(toolBarX)
    }

    getButtonFromDrawer(drawer: BaseShapeDrawer) {
        if (this.shapeButtonMap.has(drawer)) {
            return this.shapeButtonMap.get(drawer)
        }
        Logger.error("Can't find drawer for button:" + drawer.name)
        return null;
    }

    onEndOfDrawingShape(drawer: BaseShapeDrawer) {
        if (drawer) {
            let button = this.getButtonFromDrawer(drawer)
            this.setButtonBackgroundColor(button, false)
        }
    }

    onDrawShapeBegins(newDrawer: BaseShapeDrawer) {
        if (this.currentActiveDrawer == newDrawer)
            return;

        if (this.currentActiveDrawer) {
            // this.currentActiveDrawer.isSelected = false;
            let currentActiveButton = this.getButtonFromDrawer(this.currentActiveDrawer)
            this.setButtonBackgroundColor(currentActiveButton, false)
        }
        this.currentActiveDrawer = newDrawer
        let currentActiveButton = this.getButtonFromDrawer(newDrawer)
        this.setButtonBackgroundColor(currentActiveButton, true)
    }

    createButton(shape: BaseShapeDrawer) {
        let img = document.createElement("img")
        if (shape.imgClass != "unknown_img")
            img.className = shape.imgClass
        else if (shape.imgCss) {
            img.src = shape.imgCss
        }

        let i18n = (window as any).i18n

        img.title = i18n.t(shape.name)
        img.style.width = "20px"
        img.style.height = "20px"


        let button = document.createElement('button')
        button.className = "btn btn-outline-secondary"
        // button.onclick = shape.onClicked.bind(shape)
        button.appendChild(img)
        this.buttonContainer.appendChild(button)

        this.shapeButtonMap.set(shape, button)
    }

}


export {DrawToolBar}