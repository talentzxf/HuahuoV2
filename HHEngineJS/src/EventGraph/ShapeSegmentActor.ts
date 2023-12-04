import {AbstractGraphAction, ActionParam, GraphAction} from "./GraphActions";
import {PropertyType} from "hhcommoncomponents";

class ShapeSegmentActor extends AbstractGraphAction{
    pointMap: Map<number, paper.Point> = new Map
    handleInMap : Map<number, paper.Point> = new Map
    handleOutMap: Map<number, paper.Point> = new Map

    isPointValid(idx){
        return this.pointMap.has(idx)
    }

    isHandleInValid(idx){
        return this.handleInMap.has(idx)
    }

    isHandleOutValid(idx){
        return this.handleOutMap.has(idx)
    }

    getPoint(idx){
        return this.pointMap.get(idx)
    }

    getHandleIn(idx){
        return this.handleInMap.get(idx)
    }

    getHandleOut(idx){
        return this.handleOutMap.get(idx)
    }

    @GraphAction()
    setPoint(@ActionParam(PropertyType.NUMBER) idx, @ActionParam(PropertyType.VECTOR2) point){
        this.pointMap.set(idx, point)
    }

    @GraphAction()
    setHandleIn(@ActionParam(PropertyType.NUMBER) idx, @ActionParam(PropertyType.VECTOR2) handleIn){
        this.handleInMap.set(idx, handleIn)
    }

    @GraphAction()
    setHandleOut(@ActionParam(PropertyType.NUMBER) idx, @ActionParam(PropertyType.VECTOR2) handleOut){
        this.handleOutMap.set(idx, handleOut)
    }
}

export {ShapeSegmentActor}