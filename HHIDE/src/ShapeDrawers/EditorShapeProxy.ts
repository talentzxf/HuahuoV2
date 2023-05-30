import {BaseShapeJS} from "hhenginejs";
import {relaxRectangle} from "hhcommoncomponents"
import {getMethodsAndVariables} from "hhcommoncomponents"
import {defaultShapeDrawer} from "./Shapes"

const BOUNDMARGIN: number = 10

class BaseShapeHandler{
    boundingBoxGroup: paper.Group = null

    targetShape:BaseShapeJS

    functionMap: Map<string, Function> = new Map()

    proxy
    setProxy(proxy){
        this.proxy = proxy
    }

    constructor(targetShape) {
        this.targetShape = targetShape

        let _this = this
        getMethodsAndVariables(this, true).forEach((key)=>{
            if(typeof _this[key] == "function" &&
                key != "constructor" &&
                !key.startsWith("__") &&
                key != "get"){
                _this.functionMap.set(key, _this[key].bind(_this))
            }
        })
    }

    removePaperObj(){
        this.targetShape.removePaperObj.apply(this.proxy)
        this.updateBoundingBox()
    }

    setText(inText: string){
        this.targetShape.setText.apply(this.proxy, [inText])
        this.updateBoundingBox()
    }

    store(){
        this.targetShape.store.apply(this.proxy)
        this.updateBoundingBox()
    }

    createShape(){
        this.targetShape.createShape.apply(this.proxy)
        this.targetShape.paperShape.data.meta = this.proxy
    }

    removeSegment(segment){
        this.targetShape.removeSegment.apply(this.proxy, [segment])
        this.updateBoundingBox()
    }

    update(force: boolean = false){
        this.targetShape.update.apply(this.proxy, [force])
        this.updateBoundingBox()
    }

    hide(){
        this.targetShape.hide.apply(this.proxy)
        this.updateBoundingBox()
    }

    updateBoundingBox() {
        let targetShape = this.targetShape

        if (targetShape.isSelected) {
            {
                if (this.boundingBoxGroup)
                    this.boundingBoxGroup.remove()

                let boundingBox = targetShape.paperItem.bounds;

                let paperjs = targetShape.getPaperJs()
                this.boundingBoxGroup = new paperjs.Group()

                let rotationStickLength = 50;
                let rotationHandleRadius = 10;
                let xyDiff = rotationStickLength/Math.sqrt(2)

                let boundingBoxRectangle:paper.Rectangle = relaxRectangle(boundingBox, BOUNDMARGIN)
                let boundingBoxRect = new paperjs.Path.Rectangle(boundingBoxRectangle)
                boundingBoxRect.dashArray = [4, 10]
                boundingBoxRect.strokeColor = new paper.Color("black")
                this.boundingBoxGroup.addChild(boundingBoxRect)

                // Draw rotation handles.
                // 0 -- top-left
                // 1 -- top-right
                // 2 -- bottom-right
                // 3 -- bottom-left
                for(let dir = 0; dir <= 3; dir++){
                    let startPoint = null;
                    let endPoint = null;

                    switch(dir){
                        case 0:
                            startPoint = boundingBoxRectangle.topLeft
                            endPoint = startPoint.add(new paper.Point(-xyDiff, -xyDiff))
                            break;
                        case 1:
                            startPoint = boundingBoxRectangle.topRight
                            endPoint = startPoint.add(new paper.Point(xyDiff, -xyDiff))
                            break;
                        case 2:
                            startPoint = boundingBoxRectangle.bottomRight
                            endPoint = startPoint.add(new paper.Point(xyDiff, xyDiff))
                            break;
                        case 3:
                            startPoint = boundingBoxRectangle.bottomLeft
                            endPoint = startPoint.add(new paper.Point(-xyDiff, xyDiff))
                            break;
                    }

                    let line = new paperjs.Path.Line(startPoint, endPoint)
                    line.dashArray = [4,10]
                    line.strokeColor = new paper.Color("black")
                    this.boundingBoxGroup.addChild(line)

                    // Draw circle at endPoint
                    let rotationIndicatorCircle = new paperjs.Path.Circle(endPoint, rotationHandleRadius)
                    rotationIndicatorCircle.strokeColor = new paper.Color("black")
                    rotationIndicatorCircle.fillColor = new paper.Color("lightblue")
                    this.boundingBoxGroup.addChild(rotationIndicatorCircle)

                    rotationIndicatorCircle.onMouseEnter = defaultShapeDrawer.onShowRotationIndicator.bind(defaultShapeDrawer)
                    rotationIndicatorCircle.onMouseLeave = defaultShapeDrawer.onHideRotationIndicator.bind(defaultShapeDrawer)
                }

                let centerCircle = new paperjs.Path.Circle(targetShape.pivotPosition, 10)
                centerCircle.fillColor = new paper.Color("red")
                centerCircle.data.meta = targetShape.shapeCenterSelector

                this.boundingBoxGroup.addChild(centerCircle)
            }

            if (targetShape.paperItem)
                targetShape.paperItem.selected = true
        } else {
            if (targetShape.paperItem)
                targetShape.paperItem.selected = false
            if (this.boundingBoxGroup)
                this.boundingBoxGroup.remove()
            if (targetShape.shapeCenterSelector)
                targetShape.shapeCenterSelector.selected = false
        }
    }

    get(target, propKey, receiver){
        const origProperty = target[propKey]

        let _this = this

        if(origProperty instanceof Function){
            return function(...args){
                if(!_this.functionMap.has(origProperty.name)){
                    return origProperty.apply(this, args)
                }

                return _this.functionMap.get(origProperty.name).apply(this, args)
            }
        }

        return origProperty
    }
}

class EditorShapeProxy extends BaseShapeJS{

    targetObj
    static CreateProxy(baseShape: BaseShapeJS){
        let proxyHandler = new BaseShapeHandler(baseShape)
        let proxy = new Proxy(baseShape, proxyHandler)
        proxyHandler.setProxy(proxy)

        return proxy
    }
}

export {EditorShapeProxy}