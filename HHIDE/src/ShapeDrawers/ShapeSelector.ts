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
import {HHContent} from "hhpanel"
import {findParentContent} from "hhpanel";
import {objectDeleter} from "./ObjectDeleter";
import {clearPrompt, setPrompt} from "../init";


const BOUNDMARGIN: number = 10
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
    selectedSegment: paper.Segment = null

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

    sendSelectedToBack() {
        for (let shape of this.selectedShapes) {
            shape.sendToBack()
        }
    }

    bringToFront() {
        for (let shape of this.selectedShapes) {
            shape.bringToFront()
        }
    }

    duplicateShape() {
        console.log("Trying to duplicate shape")
        for (let shape of this.selectedShapes) {
            console.log("Duplicating shape")
            let duplicatedShape = shape.duplicate();

            // Offset the shape a little to avoid covering the original shape.
            let position = duplicatedShape.position
            position.x += Math.floor( Math.random() * 5 - 10 )
            position.y += Math.floor( Math.random() * 5 - 10 )

            duplicatedShape.position = position

            duplicatedShape.store()
        }
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
        this.canvas = canvas
        this.clearSelection()

        let _this = this

        let i18n = (window as any).i18n

        i18n.ExecuteAfterInited(() => {

            // setup right click context menu
            if (!_this.contextMenuInitedMap.get(canvas)) {
                _this.canvas.addEventListener("contextmenu", _this.contextMenu.onContextMenu.bind(_this.contextMenu))

                _this.contextMenu.setItems([
                    {
                        itemName: i18n.t("contextmenu.sendToBack"),
                        onclick: _this.sendSelectedToBack.bind(_this)
                    },
                    {
                        itemName: i18n.t("contextmenu.bringToFront"),
                        onclick: _this.bringToFront.bind(_this)
                    },
                    {
                        itemName: i18n.t("contextmenu.duplicate"),
                        onclick: _this.duplicateShape.bind(_this)
                    },
                    {
                        itemName: i18n.t("contextmenu.createNewElement"),
                        onclick: elementCreator.onNewElement.bind(elementCreator)
                    },
                    {
                        itemName: i18n.t("contextmenu.groupAsElement"),
                        onclick: _this.groupAsElement.bind(_this)
                    },
                    {
                        itemName: i18n.t("contextmenu.delete"),
                        onclick: _this.deleteSelectedObj.bind(_this)
                    }
                ])

                // Setup other short cuts.
                let parentContent = findParentContent(_this.canvas)
                parentContent.addEventListener('keydown', _this.onKeyDown.bind(this))

                _this.contextMenuInitedMap.set(_this.canvas, true)
            }
        })
        EventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onShapeSelected.bind(this))
    }

    groupAsElement(){
        // Calculate the center of all the selected elements.
        let newCenterPosition = new paper.Point(0,0)
        for(let shape of this.selectedShapes){
            let shapeCenter = shape.shapePosition
            newCenterPosition.x += shapeCenter.x
            newCenterPosition.y += shapeCenter.y
        }

        newCenterPosition.x /= this.selectedShapes.size
        newCenterPosition.y /= this.selectedShapes.size

        let element = elementCreator.createElement(this.selectedShapes)

        this.selectedShapes.clear()
        if(element)
            this.selectedShapes.add(element) // Only select this element

        element.pivotPosition = newCenterPosition
    }

    onShapeSelected(property, targetObj: any) {
        console.log("Something selected")
        if (targetObj instanceof paper.Segment) {
            if (this.selectedSegment && this.selectedSegment != targetObj) {
                this.selectedSegment.selected = false
                this.selectedSegment.handleIn.selected = false
                this.selectedSegment.handleOut.selected = false
            }

            this.selectedSegment = targetObj
        }
    }


    deleteSelectedObj() {
        if (this.selectedSegment) {
            objectDeleter.deleteSegment(this.selectedSegment)

            this.selectedSegment = null
        } else {
            for (let shape of this.selectedShapes) {
                objectDeleter.deleteShape(shape)
            }

            this.clearSelection(false)
        }
    }

    onKeyDown(e: KeyboardEvent) {
        let targetContent = e.target as HHContent
        if (!targetContent.getAttribute("selected"))
            return

        let handled = false
        if (e.ctrlKey && e.code == 'KeyD') {
            this.duplicateShape()
            handled = true
        } else if (e.code == "Delete") {
            handled = true

            this.deleteSelectedObj()
        }

        if (handled) {
            e.preventDefault()
            e.stopPropagation()
        }
    }

    // Find the outmost parent of the shape (Will change to find the correct hierarchy later
    findParentOf(shape: BaseShapeJS) {
        let parent = shape.getParent()
        let itr = shape

        while (parent != null) {
            itr = parent
            parent = parent.getParent()
        }

        return itr
    }

    hitSomething(scrX, scrY, clearSelection: boolean = false): boolean {
        let hitPoint = BaseShapeDrawer.getWorldPosFromView(scrX, scrY)
        // Single click, perform hit test.
        let hitResultArray = paper.project.hitTestAll(hitPoint, this.hitOptions)

        for(let hitResult of hitResultArray){
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
                        if(this.hitTypeSeletable(hitResult.item, hitType)){
                            this.transformHandler = this.transformHandlerMap.getHandler(hitType)

                            if (this.transformHandler)
                                this.setTransformHandler(this.selectedShapes, hitPoint, this.transformHandler, hitResult)
                        }
                    }

                    return true
                }
            }
        }

        EventBus.getInstance().emit(EventNames.UNSELECTOBJECTS)
        return false
    }

    selectObject(shape: BaseShapeJS) {
        let selectedObj = shape
        selectedObj.selected = true
        selectedObj.update(true);
        this.selectedShapes.add(shape)

        EventBus.getInstance().emit(EventNames.OBJECTSELECTED, selectedObj.getPropertySheet(), selectedObj)

        setPrompt(i18n.t("statusbar.selectShape", {shapeType: shape.getTypeName(), selectedShapeCount: this.selectedShapes.size}))
    }

    clearSelection(updateSelectedShapes: boolean = true) {
        if (updateSelectedShapes) {
            // 1. Clear current selections. TODO: How about multiple selection ???
            for (let shape of this.selectedShapes) {
                shape.selected = false
                shape.update(true)
            }
        }

        this.selectedShapes = new Set()
        this.selectedSegment = null
        this.transformHandler = null
        this.canvas.style.cursor = "default"

        clearPrompt()
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
        if (this.selectedShapes.size != 1) {
            this.canvas.style.cursor = "default"
            this.transformHandler = null
            return
        }

        let targetPos = new paper.Point(pos.x, pos.y)

        let targetShape = this.selectedShapes.values().next().value
        if(targetShape == null || targetShape.paperShape == null){
            this.selectedShapes.delete(targetShape)
            return
        }

        let bounds = relaxRectangle(targetShape.paperShape.bounds, BOUNDMARGIN)

        if (!bounds.contains(targetPos)) {
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

                for (let shape of this.selectedShapes) {
                    elementCreator.dispatchElementChange(shape.getBornStoreId())
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
        if (item.data.meta == null || typeof item.data.meta == "undefined" || false == item.data.meta.isSelectable())
            return false
        return true
    }

    hitTypeSeletable(item, type){
        if(!this.itemSelectable(item)) // Defensive coding, check whether the item is hitable again
            return false

        return item.data.meta.hitTypeSelectable(type)
    }

    setTransformHandler(targetObjs: Set<BaseShapeJS>, pos: Vector2, handler: ShapeTranslateMorphBase = TransformHandlerMap.defaultTransformHandler, hitResult = null) {
        this.transformHandler = handler

        this.transformHandler.setTarget(targetObjs)
        this.transformHandler.beginMove(pos, hitResult)
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        if (this.transformHandler) {
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
                            if(this.itemSelectable(shape)){
                                this.selectObject(shape.data.meta)
                                this.setTransformHandler(this.selectedShapes, pos)
                            }
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
        if (this.selectedShapes.size == 1) {
            let selectedShape = this.selectedShapes.values().next().value
            if (selectedShape.getTypeName() == "ElementShape") {
                elementCreator.openElementEditTab(selectedShape)
            }
        }
    }
}

export {ShapeSelector}