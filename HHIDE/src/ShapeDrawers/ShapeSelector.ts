import {BaseShapeJS} from "hhenginejs"
import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {Vector2, pointsNear, relaxRectangle, ContextMenu} from "hhcommoncomponents"
import {paper} from "hhenginejs";
import {shapeScaleHandler} from "../TransformHandlers/ShapeScaleHandler";
import {ShapeTranslateMorphBase} from "../TransformHandlers/ShapeTranslateMorphBase";
import {TransformHandlerMap} from "../TransformHandlers/TransformHandlerMap";
import {shapeRotateHandler} from "../TransformHandlers/ShapeRotateHandler";
import {EventBus, EventNames} from "../Events/GlobalEvents";
import {elementCreator} from "../SceneView/ElementCreator";
import {huahuoEngine} from "hhenginejs";


const BOUNDMARGIN:number = 10
const VERYNEARMARGIN = 10
const NEARBOUNDMARGIN = 25

class ShapeSelector extends BaseShapeDrawer {
    selectRectangle: paper.Path.Rectangle;
    imgClass = "fas fa-arrow-pointer"
    name = "ShapeSelector"

    startPos: Vector2;
    margin: number = 5;
    hitOptions = {}

    selectedShapes: Set<BaseShapeJS> = new Set()

    transformHandler: ShapeTranslateMorphBase = null
    canvas: HTMLCanvasElement
    transformHandlerMap: TransformHandlerMap
    private contextMenu: ContextMenu = new ContextMenu()
    private contextMenuInitedMap: Map<HTMLCanvasElement, boolean> = new Map();

    constructor() {
        super();

        this.hitOptions = {
            segments: true,
            stroke: true,
            fill: true,
            handles: true,
            tolerance: 5
        }

        this.transformHandlerMap = new TransformHandlerMap()
    }

    isDefaultDrawer(): boolean {
        return true
    }

    sendSelectedToBack(){
        for(let shape of this.selectedShapes){
            shape.sendToBack()
        }
    }

    bringToFrond(){
        for(let shape of this.selectedShapes){
            shape.bringToFront()
        }
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        this.canvas = canvas
        this.clearSelection()

        // setup right click context menu
        if(!this.contextMenuInitedMap.get(canvas)){
            this.canvas.addEventListener("contextmenu", this.contextMenu.onContextMenu.bind(this.contextMenu))

            let _this = this
            this.contextMenu.setItems([
                {
                    itemName: "Send To Back",
                    onclick: _this.sendSelectedToBack.bind(_this)
                },
                {
                    itemName: "Bring to Front",
                    onclick: _this.bringToFrond.bind(_this)
                },
                {
                    itemName: "Create New Element",
                    onclick: elementCreator.onNewElement.bind(elementCreator)
                }
                ])

            this.contextMenuInitedMap.set(this.canvas, true)
        }
    }

    // Find the outmost parent of the shape (Will change to find the correct hierarchy later
    findParentOf(shape:BaseShapeJS){
        let parent = shape.getParent()
        let itr = shape

        while(parent != null){
            itr = parent
            parent = parent.getParent()
        }

        return itr
    }

    hitSomething(scrX, scrY, clearSelection: boolean = false): boolean {
        let hitPoint = BaseShapeDrawer.getWorldPosFromView(scrX, scrY)
        // Single click, perform hit test.
        let hitResult = paper.project.hitTest(hitPoint, this.hitOptions)
        if (hitResult) {
            let hitItem = hitResult.item;
            if (this.itemSelectable(hitResult.item)) {
                console.log("HitType:" + hitResult.type)

                if (!hitItem.data.meta.selected) { // If the object has not been selected, select it.
                    if (clearSelection) {
                        this.clearSelection()
                    }

                    this.selectObject(this.findParentOf(hitItem.data.meta))
                    this.setTransformHandler(this.selectedShapes, hitPoint)
                } else { // If the object has already been selected, we might need to do something based on the hittype.

                    let hitType = hitResult.type
                    this.transformHandler = this.transformHandlerMap.getHandler(hitType)

                    if (this.transformHandler)
                        this.setTransformHandler(this.selectedShapes, hitPoint, this.transformHandler, hitResult)
                }

                return true
            }
        }

        EventBus.getInstance().emit(EventNames.UNSELECTOBJECTS)
        return false
    }

    selectObject(shape: BaseShapeJS) {
        let selectedObj = shape
        selectedObj.selected = true
        selectedObj.update();
        this.selectedShapes.add(shape)

        EventBus.getInstance().emit(EventNames.OBJECTSELECTED, selectedObj.getPropertySheet())
    }

    clearSelection() {
        // 1. Clear current selections. TODO: How about multiple selection ???
        for (let shape of this.selectedShapes) {
            shape.selected = false
            shape.update()
        }
        this.selectedShapes = new Set()
        this.transformHandler = null
        this.canvas.style.cursor = "default"
    }

    onMouseDown(evt: MouseEvent) {
        if (evt.buttons != 1)
            return
        super.onMouseDown(evt);

        let pos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        if (this.transformHandler) {
            this.transformHandler.beginMove(pos)
        } else {
            // 1. Hit testing.
            if (!this.hitSomething(evt.offsetX, evt.offsetY, !evt.ctrlKey)) { // Ctrl key was pressed and hit something else.
                this.clearSelection()

                // 2. Didn't hit anything, begin to draw select box.
                this.isDrawing = true;
                this.startPos = pos
            }
        }
    }

    showRotateScaleCursor(pos: Vector2) {
        if (this.selectedShapes.size != 1){
            this.canvas.style.cursor = "default"
            this.transformHandler = null
            return
        }

        let targetPos = new paper.Point(pos.x, pos.y)

        let targetShape = this.selectedShapes.values().next().value
        let bounds = relaxRectangle(targetShape.paperShape.bounds, BOUNDMARGIN)

        if(!bounds.contains(targetPos)){
            if (pointsNear(bounds.topLeft, targetPos, VERYNEARMARGIN)) {
                this.canvas.style.cursor = "nw-resize"
                this.setTransformHandler(this.selectedShapes, pos, shapeScaleHandler)
                return;
            } else if (pointsNear(bounds.topRight, targetPos, VERYNEARMARGIN)) {
                this.canvas.style.cursor = "ne-resize"
                this.setTransformHandler(this.selectedShapes, pos, shapeScaleHandler)
                return;
            } else if (pointsNear(bounds.bottomLeft, targetPos, VERYNEARMARGIN)) {
                this.canvas.style.cursor = "nesw-resize"
                this.setTransformHandler(this.selectedShapes, pos, shapeScaleHandler)
                return;
            } else if (pointsNear(bounds.bottomRight, targetPos, VERYNEARMARGIN)) {
                this.canvas.style.cursor = "nwse-resize"
                this.setTransformHandler(this.selectedShapes, pos, shapeScaleHandler)
                return;
            } else {
                // TODO: Add rotate handler ...
                if (pointsNear(bounds.topLeft, pos, NEARBOUNDMARGIN) ||
                    pointsNear(bounds.topRight, pos, NEARBOUNDMARGIN) ||
                    pointsNear(bounds.bottomLeft, pos, NEARBOUNDMARGIN) ||
                    pointsNear(bounds.bottomRight, pos, NEARBOUNDMARGIN)) {
                    this.canvas.style.cursor = "url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAETUlEQVR42t2VfUzVVRjHn+ecH/d6r1zeLPWiEQHXF2CmUaM/iOboZdVyQjPiJS2dTVKDiqm1UCuXTcduuSz7I3pfUc6WtIXp0A1XqNCi5CZw1TkFBORyudf78ns9nd+PexGqJdT8p7M9e3bO75zv5zzPec75Idzghv8fQHNzi12W5ZmSJIIoSnDNS6BpGlBKuNGoMVVVXWVlxcqkAcePn6i12azLuBjyxRA1TWMgCAISDmC8QwgBj8cz7PP5K1asKGqdCsDpcKRW6QKIyHC0gclkYleCArr6EZKsANl2ZO6uM+e6u90lHHBqSoD589MqdVHddJCeDqQCNHURyLGHwC9RcA+bgVztuTwyeLGwdPm9J6YEWLgwoyoC0FOBPDXwax9lTBExNVEBwr8BEnbBK+DpPtoRlMnzGwvMh/X1uxrFKv51IBj0128rukn9W0BmpsNIETfGd49hlcLBdpUVpAWREj0qIzIOR5Q1wtp7BXD1k4OyAtWSChn8uGr4FE1StEd2FFp9fwKcdGZnz6scVynwbTvArTY/2OO0qLgRRWQToEO9YQLNZ6l0fgjellW2MyTDm8DAVFtsWT0BsKWu89P8JcnlSxcIRhn2jBBsbA/Ag/OCjHL1caJG+qgeCcXoOHMPAh5yYW+PV8sPivBTOBxI/7ziZj/essGNcsyMGlGGrVnJhDRVW4EKFGoPiXDfbV5IsEDkwIkRBdUF9TsRAeoRYWT8/WYWcvVp6Veusq/5zqu+2RDbhpmvjNTLKtyuapAKwMzOJ6yQaEV2+rwHl6aL/A5Qfh7GjiMQEo2Ie0QjnQJhv1xCdB5RGgZ8WilHfseZ1cc221pxcc1AChFMwwJBz+7HrcK7R0WYFauxVYuH0ByDINBRgGCcCxnfZ1zYAPDrxzYfUC62XVD3+ELs0RgKGTzrz/LpGcYZ5D7XlJKcsaTrwMYE875jIljUIZYzV+Yixl1gAh0VGu0TxqNC3euVpo/t/5mx+lNK2blBbf9QgLkELszNaxLgKQNQsOlo0qz0O/vfKrHSTV964YV7fBExOtELY1GMeU8QYfXH0o+HX7Tl6VrT13lKee28xMUfG34nsWusip75JNj9sMMX/1ELxpXmyKZFySyaluiO/5oi7rc3KOqR39W727bGGe+SufgkQetsS/jDlMCEMq3+KlRhodI2TfStbThjy1tkVyrX5yvmOQlk9JCvRRNNGfuth+HKOrGu8434Ndd9KvT2WkPYyQCeDIjsvRa3XH92QN2xKldbtjYPIXbaxJTpJbp8b9jX0assuFSbdHlSAL3tbgzn8rfo6b7+wfW1K+eqses899vj0fnyQySr8A5BB/AIKNa3qqzyi/AW797EXZN67P6pTSvvEMj05IrcNNy+s8ic6Jgt4F2vB9x9Q4Gs0AdzpP8MiDbLmt4Z1GR51TGTlHf2qyXBfUnfX2/Nv/onmx74jEg/lGuTmXvDf/p/AC99yhRBi1wxAAAAAElFTkSuQmCC'), auto"
                    this.setTransformHandler(this.selectedShapes, pos, shapeRotateHandler)
                    return;
                }
            }
        }

        this.canvas.style.cursor = "default"
        this.transformHandler = null

    }

    onMouseMove(evt: MouseEvent) {
        let pos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

        if (evt.buttons != 1) {
            this.showRotateScaleCursor(pos);
        } else {
            super.onMouseMove(evt);

            if (this.transformHandler && this.transformHandler.getIsDragging()) {
                this.transformHandler.dragging(pos)

                for(let shape of this.selectedShapes){
                    let targetStoreId = shape.getBornStoreId()

                    while(targetStoreId){
                        elementCreator.dispatchElementChange(targetStoreId)
                        targetStoreId = huahuoEngine.getElementParentByStoreId(targetStoreId)
                    }
                }
            } else {
                if (this.isDrawing) {

                    let endPos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

                    if (this.selectRectangle) {
                        this.selectRectangle.remove()
                    }

                    this.selectRectangle = new paper.Path.Rectangle(this.startPos, endPos)
                    this.selectRectangle.strokeColor = new paper.Color("Black")
                    this.selectRectangle.dashArray = [10, 12];
                }
            }
        }
    }

    itemSelectable(item) {
        if (typeof item.data.meta == "undefined" || false == item.data.meta.isSelectable())
            return false
        return true
    }

    setTransformHandler(targetObjs: Set<BaseShapeJS>, pos: Vector2, handler: ShapeTranslateMorphBase = TransformHandlerMap.defaultTransformHandler, hitResult = null) {
        this.transformHandler = handler
        this.transformHandler.setTarget(targetObjs)
        this.transformHandler.beginMove(pos, hitResult)
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        if(this.transformHandler){
            this.transformHandler.endMove();
            this.transformHandler = null;
        }

        this.isDrawing = false

        if (this.selectRectangle) {
            let pos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
            // Test against all shapes.
            // TODO: any faster algorithm?  AABB? Oct-Tree?
            for (let layer of paper.project.layers) {
                for (let shape of layer.children) {
                    if (shape.data && shape.data.meta && shape.data.meta.isVisible()) {
                        let shapeBoundingBox = shape.getBounds()
                        let selectionRectBoundingBox = this.selectRectangle.getBounds()
                        if (shapeBoundingBox.intersects(selectionRectBoundingBox)) {
                            this.selectObject(shape.data.meta)
                            this.setTransformHandler(this.selectedShapes, pos)
                        }
                    }
                }
            }
            this.selectRectangle.remove()
            this.selectRectangle = null
        }
    }


    onDblClick(evt: MouseEvent) {
        super.onDblClick(evt);
        if(this.selectedShapes.size == 1){
            let selectedShape = this.selectedShapes.values().next().value
            if(selectedShape.getTypeName() == "ElementShape"){
                elementCreator.openElementEditTab(selectedShape)
            }
        }
    }
}

export {ShapeSelector}