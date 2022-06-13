import {Logger, Vector2} from "hhcommoncomponents";
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {renderEngine2D} from "hhenginejs"

class BaseShapeDrawer{
    name = "unknown_shape"
    imgClass = "unknown_img"

    isDrawing = false
    isSelected = false

    canvasWidth = -1;
    canvasHeight = -1;

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

    getWorldPosFromView(x, y): Vector2{
        return renderEngine2D.getWorldPosFromView(x,y)
    }

    onMouseUp(evt: MouseEvent){
    }

    onMouseMove(evt: MouseEvent){

    }
}

export {BaseShapeDrawer}