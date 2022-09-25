import {Logger, Vector2} from "hhcommoncomponents";
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {huahuoEngine, renderEngine2D} from "hhenginejs"
import {BaseShapeJS} from "hhenginejs";
import {elementCreator} from "../SceneView/ElementCreator";

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

        EventBus.getInstance().emit(EventNames.DRAWSHAPEBEGINS, this)
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
        let currentLayer = huahuoEngine.GetCurrentLayer()
        currentLayer.addShape(shape)

        let targetStoreId = shape.getBornStoreId()

        while(targetStoreId){
            elementCreator.dispatchElementChange(targetStoreId)
            targetStoreId = huahuoEngine.getElementParentByStoreId(targetStoreId)
        }
    }

    onMouseUp(evt: MouseEvent){
    }

    onMouseMove(evt: MouseEvent){
        
    }

    onDblClick(evt: MouseEvent){

    }
}

export {BaseShapeDrawer}