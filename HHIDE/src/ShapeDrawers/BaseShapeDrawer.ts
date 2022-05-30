import {Logger} from "hhcommoncomponents";
import {TypedEmitter} from "tiny-typed-emitter";

enum BaseShapeEventNames{
    DRAWSHAPEBEGINS = 'drawShapeBegins'
}

interface ShapeEvents{
    drawShapeBegins:(shapeDrawer: BaseShapeDrawer)=>void;
}

class BaseShapeDrawer extends TypedEmitter<ShapeEvents>{
    name = "unknown_shape"
    imgClass = "unknown_img"

    isDrawing = false
    isSelected = false

    onClicked(isSelected){
        Logger.Debug(this.name + " clicked!")
        this.emit(BaseShapeEventNames.DRAWSHAPEBEGINS, this)
        if(isSelected)
            this.isSelected = true
    }

    onDrawShape(){
    }

    onMouseDown(){

    }

    onMouseUp(){
        this.isSelected = false
    }

    onMouseMove(){

    }

    getBackgroundColor(){
        if(this.isSelected)
            return "#42b983";
        return 'white';
    }
}

export {BaseShapeDrawer}