import {Vector2} from "hhcommoncomponents"

class CurvePoint{
    position: Vector2
    handleIn: Vector2
    handleOut: Vector2
}

class CurveArray{
    array: Array<CurvePoint> = new Array<CurvePoint>()
}

class CurveUtils{
    static DrawLine(startPoint: Vector2, endPoint:Vector2){
    }
}

export {CurveUtils}