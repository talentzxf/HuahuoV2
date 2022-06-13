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

    selectedShapes: Array<BaseShapeJS> = new Array()


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

    hitSomething(scrX, scrY): boolean {
        let hitPoint = BaseShapeDrawer.getWorldPosFromView(scrX, scrY)
        // Single click, perform hit test.
        let hitResult = paper.project.hitTest(hitPoint, this.hitOptions)
        if (hitResult) {
            let hitItem = hitResult.item;
            if (this.itemSelectable(hitResult.item)) {

                let selectedObj = hitItem.data.meta
                selectedObj.selected = true
                selectedObj.update();

                this.setTransformHandler(selectedObj, hitPoint)

                this.selectedShapes.push(hitItem.data.meta)

                return true
            }
        }
        return false
    }

    onMouseDown(evt: MouseEvent) {
        if (evt.buttons != 1)
            return
        super.onMouseDown(evt);

        // 1. Clear current selections. TODO: How about multiple selection ???
        for (let shape of this.selectedShapes) {
            shape.selected = false
            shape.update()
        }

        this.selectedShapes = new Array()

        // 2. Hit testing. If anything was hit.
        if (this.hitSomething(evt.offsetX, evt.offsetY)) {
            return
        }

        // 3. Did hit anything, begin to draw select box.
        this.isDrawing = true;
        this.startPos = BaseShapeDrawer.getWorldPosFromView(evt.offsetX, evt.offsetY)
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

    setTransformHandler(targetObj: BaseShapeJS, pos: Vector2) {
        this.transformHandler = this.defaultTransformHandler
        this.transformHandler.setTarget(targetObj)
        this.transformHandler.beginMove(pos)
    }

    onMouseUp(evt: MouseEvent) {
        if (evt.buttons != 1)
            return

        super.onMouseUp(evt);

        this.transformHandler = null;

        this.isDrawing = false

        if (this.selectRectangle)
            this.selectRectangle.remove()
    }
}

export {ShapeSelector}