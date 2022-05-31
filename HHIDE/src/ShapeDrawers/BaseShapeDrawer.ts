import {Logger} from "hhcommoncomponents";
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {Vector2} from "../Math/Vector2";

class BaseShapeDrawer{
    name = "unknown_shape"
    imgClass = "unknown_img"

    isDrawing = false
    isSelected = false

    canvasWidth = -1;
    canvasHeight = -1;

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

    getRelativePosition(x, y): Vector2{
        return new Vector2(x/this.canvasWidth, y/this.canvasHeight)
    }

    onMouseUp(evt: MouseEvent){
    }

    onMouseMove(evt: MouseEvent){

    }
}

export {BaseShapeDrawer}