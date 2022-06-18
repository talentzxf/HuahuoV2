import {BaseShapeJS} from "hhenginejs"
import {BaseShapeDrawer} from "./BaseShapeDrawer";
import {Vector2} from "hhcommoncomponents"
import {paper} from "hhenginejs";
import {shapeTranslateHandler} from "../TransformHandlers/ShapeTranslateHandler";
import {ShapeTranslateMorphBase} from "../TransformHandlers/ShapeTranslateMorphBase";

class ShapeSelector extends BaseShapeDrawer {
    selectRectangle: paper.Path.Rectangle;
    imgClass = "fas fa-arrow-pointer"
    name = "ShapeSelector"

    startPos: Vector2;
    margin: number = 5;
    hitOptions = {}

    selectedShapes: Set<BaseShapeJS> = new Set()


    transformHandler: ShapeTranslateMorphBase = null
    defaultTransformHandler: ShapeTranslateMorphBase = shapeTranslateHandler

    constructor() {
        super();

        this.hitOptions = {
            segments: true,
            stroke: true,
            fill: true,
            handles: true,
            tolerance: 5
        }
    }

    isDefaultDrawer(): boolean {
        return true
    }

    onBeginToDrawShape(canvas: HTMLCanvasElement) {
        super.onBeginToDrawShape(canvas);
    }

    hitSomething(scrX, scrY, clearSelection: boolean = false): boolean {
        let hitPoint = BaseShapeDrawer.getWorldPosFromView(scrX, scrY)
        // Single click, perform hit test.
        let hitResult = paper.project.hitTest(hitPoint, this.hitOptions)
        if (hitResult) {
            let hitItem = hitResult.item;
            if (this.itemSelectable(hitResult.item)) {
                if(!hitItem.data.meta.selected){
                    if (clearSelection) {
                        this.clearSelection()
                    }

                    this.selectObject(hitItem.data.meta, hitPoint)
                }else{
                    if(this.transformHandler){
                        this.transformHandler.beginMove(hitPoint)
                    }
                }

                return true
            }
        }
        return false
    }

    selectObject(shape: BaseShapeJS, hitPoint: Vector2) {
        let selectedObj = shape
        selectedObj.selected = true
        selectedObj.update();
        this.selectedShapes.add(shape)

        this.setTransformHandler(this.selectedShapes, hitPoint)
    }

    clearSelection() {
        // 1. Clear current selections. TODO: How about multiple selection ???
        for (let shape of this.selectedShapes) {
            shape.selected = false
            shape.update()
        }
        this.selectedShapes = new Set()
        this.transformHandler = null
    }

    onMouseDown(evt: MouseEvent) {
        if (evt.buttons != 1)
            return
        super.onMouseDown(evt);

        // 1. Hit testing.
        if (!this.hitSomething(evt.offsetX, evt.offsetY, !evt.ctrlKey)) { // Ctrl key was pressed and hit something else.
            this.clearSelection()

            // 2. Didn't hit anything, begin to draw select box.
            this.isDrawing = true;
            this.startPos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
        }
    }

    onMouseMove(evt: MouseEvent) {
        if (evt.buttons != 1)
            return

        super.onMouseMove(evt);
        let pos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)

        if (this.transformHandler && this.transformHandler.getIsDragging()) {
            this.transformHandler.dragging(pos)
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

    itemSelectable(item) {
        if (typeof item.data.meta == "undefined" || false == item.data.meta.isSelectable())
            return false
        return true
    }

    setTransformHandler(targetObjs: Set<BaseShapeJS>, pos: Vector2) {
        this.transformHandler = this.defaultTransformHandler
        this.transformHandler.setTarget(targetObjs)
        this.transformHandler.beginMove(pos)
    }

    onMouseUp(evt: MouseEvent) {
        super.onMouseUp(evt);

        if (this.selectedShapes.size == 0)
            this.transformHandler = null;

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
                            this.selectObject(shape.data.meta, pos)
                        }
                    }
                }
            }
            this.selectRectangle.remove()
            this.selectRectangle = null
        }
    }
}

export {ShapeSelector}