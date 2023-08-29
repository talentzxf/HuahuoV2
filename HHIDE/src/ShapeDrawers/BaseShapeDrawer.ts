import {Logger, Vector2} from "hhcommoncomponents";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {renderEngine2D} from "hhenginejs"
import {BaseShapeJS} from "hhenginejs";
import {LayerUtils} from "../SceneView/Layer";

class BaseShapeDrawer{
    name = "unknown_shape"
    imgClass = "unknown_img"
    imgCss:string = null

    isDrawing = false
    _isSelected = false

    canvasWidth = -1;
    canvasHeight = -1;

    set isSelected(val: boolean){
        this._isSelected = val
    }

    get isSelected(): boolean{
        return this._isSelected
    }

    isDefaultDrawer(){
        return false
    }

    onClicked(isSelected){ // The button is clicked
        Logger.debug(this.name + " clicked!")
        if(isSelected)
            this.isSelected = true

        IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEBEGINS, this)
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement){
        this.canvasWidth = canvas.width
        this.canvasHeight = canvas.height
    }

    onMouseDown(evt: MouseEvent){
    }

    static getWorldPosFromView(x, y): Vector2{
        return renderEngine2D.getWorldPosFromView(x,y)
    }

    addShapeToCurrentLayer(shape:BaseShapeJS){
        LayerUtils.addShapeToCurrentLayer(shape)
    }

    onMouseUp(evt: MouseEvent){
    }

    onMouseMove(evt: MouseEvent){
        
    }

    onDblClick(evt: MouseEvent){

    }
}

export {BaseShapeDrawer}