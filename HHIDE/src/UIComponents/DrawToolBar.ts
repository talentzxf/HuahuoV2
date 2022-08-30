import {shapes} from "../ShapeDrawers/Shapes";
import {CustomElement, Logger} from "hhcommoncomponents"
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {EventBus, EventNames} from "../Events/GlobalEvents";
import i18next from "i18next";

@CustomElement({
    selector: "hh-draw-toolbar"
})
class DrawToolBar extends HTMLElement{
    private shapeButtonMap:Map<BaseShapeDrawer, HTMLButtonElement> = new Map
    private currentActiveDrawer: BaseShapeDrawer = null

    private defaultDrawer: BaseShapeDrawer = null

    setButtonBackgroundColor(button:HTMLButtonElement, isSelected:boolean){
        let bgColor = "white"
        if(isSelected)
            bgColor = "#42b983";

        button.style.backgroundColor = bgColor
    }

    initializeTools(loaded){
        for(let shape of shapes){
            this.createButton(shape)
            if(shape.isDefaultDrawer())
                this.defaultDrawer = shape;
        }

        EventBus.getInstance().on(EventNames.DRAWSHAPEBEGINS, this.onDrawShapeBegins.bind(this))
        EventBus.getInstance().on(EventNames.DRAWSHAPEENDS, this.onEndOfDrawingShape.bind(this))
    }

    connectedCallback(){
        i18next.on("loaded", this.initializeTools.bind(this))
    }

    getButtonFromDrawer(drawer: BaseShapeDrawer){
        if(this.shapeButtonMap.has(drawer)){
            return this.shapeButtonMap.get(drawer)
        }
        Logger.error("Can't find drawer for button:" + drawer.name)
        return null;
    }

    onEndOfDrawingShape(drawer: BaseShapeDrawer){
        if(drawer){
            let button = this.getButtonFromDrawer(drawer)
            this.setButtonBackgroundColor(button, false)
        }
    }

    onDrawShapeBegins(newDrawer: BaseShapeDrawer){
        if(this.currentActiveDrawer == newDrawer)
            return;

        if(this.currentActiveDrawer){
            this.currentActiveDrawer.isSelected = false;
            let currentActiveButton = this.getButtonFromDrawer(this.currentActiveDrawer)
            this.setButtonBackgroundColor(currentActiveButton, false)
        }
        this.currentActiveDrawer = newDrawer
        let currentActiveButton = this.getButtonFromDrawer(newDrawer)
        this.setButtonBackgroundColor(currentActiveButton, true)
    }

    createButton(shape: BaseShapeDrawer){
        let img = document.createElement("img")
        if(shape.imgClass != "unknown_img")
            img.className = shape.imgClass
        else if(shape.imgCss){
            img.src = shape.imgCss
        }

        img.title = i18next.t(shape.name)
        img.style.width = "20px"
        img.style.height = "20px"


        let button = document.createElement('button')
        button.onclick = shape.onClicked.bind(shape)
        button.appendChild(img)
        this.appendChild(button)

        this.shapeButtonMap.set(shape, button)
    }

}



export {DrawToolBar}