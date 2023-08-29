import {ShapeTranslateMorphBase} from "./ShapeTranslateMorphBase";
import {paper, BaseShapeJS} from "hhenginejs";
import {IDEEventBus, EventNames} from "../Events/GlobalEvents";
import {PropertySheet, PropertyType} from "hhcommoncomponents"
import {ShapeHandlerMoveCommand} from "../RedoUndo/ShapeHandlerMoveCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {ShapeSegmentMoveCommand} from "../RedoUndo/ShapeSegmentMoveCommand";
import {ShapeSegmentInsertCommand} from "../RedoUndo/ShapeSegmentInsertCommand";
import {ValueChangeHandler} from "hhenginejs";
import {setPrompt} from "../init";

class ShapeMorphHandler extends ShapeTranslateMorphBase {
    curSegment: paper.Segment
    curSegmentStartPos: paper.Point

    targetShape: BaseShapeJS

    protected pressingShift: boolean = false

    protected valueChangeHandler: ValueChangeHandler = new ValueChangeHandler()

    constructor() {
        super();

        document.body.addEventListener("keydown", this.onKeyDown.bind(this))
        document.body.addEventListener("keyup", this.onKeyUp.bind(this))
    }

    getCurSegment(){
        return this.curSegment
    }

    onKeyDown(e: KeyboardEvent) {
        if (e.shiftKey) {
            this.pressingShift = true
        }
    }

    onKeyUp(e: KeyboardEvent) {
        if (!e.shiftKey) {
            this.pressingShift = false
        }
    }

    setSegment(hitSegment: paper.Segment) {
        if (this.curSegment) {
            this.curSegment.selected = false
            this.curSegment.handleIn.selected = false
            this.curSegment.handleOut.selected = false
        }

        this.curSegment = hitSegment
        this.curSegmentStartPos = this.curSegment.point.clone()

        hitSegment.selected = true
        hitSegment.handleIn.selected = true
        hitSegment.handleOut.selected = true

    }

    beginMove(startPos, hitResult = null, showInspector: boolean = true) {
        if (this.curObjs.size != 1) {
            throw "Can't morph multiple objects!!!"
        }

        this.targetShape = this.curObjs.values().next().value // There's only one object in the set, get it.

        if (!this.targetShape.isSegmentSeletable())
            return

        super.beginMove(startPos);

        if (hitResult != null && hitResult.segment != null) {
            this.setSegment(hitResult.segment)

            if(showInspector)
                this.showInspector()
        }

        setPrompt(i18n.t("statusbar.selectedSegment"))
    }

    getPropertyGetter(propertyName: string) {
        let _this = this
        return function () {
            return _this.curSegment[propertyName]
        }
    }

    getPropertySetter(propertyName: string) {
        let _this = this
        return function (point) {
            _this.curSegment[propertyName].x = point.x
            _this.curSegment[propertyName].y = point.y

            // After morph, the position of the shape might be shifted, so we need to store the new position in the Cpp side.
            this.targetShape.store({position: true, segments: true})
        }
    }

    registerValueChangeHandler(propertyName: string) {
        return this.valueChangeHandler.registerValueChangeHandler(propertyName)
    }

    unregisterValueChangeHandler(propertyName: string) {
        return this.valueChangeHandler.unregisterValueChangeHandler(propertyName)
    }

    protected setupPropertySheet(propertySheet: PropertySheet) {
        let _this = this

        propertySheet.addProperty(
            {
                key: "point",
                type: PropertyType.VECTOR2,
                getter: this.getPropertyGetter("point").bind(this),
                setter: this.getPropertySetter("point").bind(this),
                registerValueChangeFunc: (func)=>{
                    _this.registerValueChangeHandler("point")((segment)=>{
                        func(segment.point)
                    })
                },
                unregisterValueChangeFunc: this.unregisterValueChangeHandler("point")
            })

        propertySheet.addProperty({
            key: "handleIn",
            type: PropertyType.VECTOR2,
            getter: this.getPropertyGetter("handleIn").bind(this),
            setter: this.getPropertySetter("handleIn").bind(this),
            registerValueChangeFunc: this.registerValueChangeHandler("handleIn"),
            unregisterValueChangeFunc: this.unregisterValueChangeHandler("handleIn")
        })
        propertySheet.addProperty(
            {
                key: "handleOut",
                type: PropertyType.VECTOR2,
                getter: this.getPropertyGetter("handleOut").bind(this),
                setter: this.getPropertySetter("handleOut").bind(this),
                registerValueChangeFunc: this.registerValueChangeHandler("handleOut"),
                unregisterValueChangeFunc: this.unregisterValueChangeHandler("handleOut")
            })

        propertySheet.addProperty({
            key: "Smooth",
            type: PropertyType.BUTTON,
            config: {
                action: this.smoothSegment.bind(this)
            }
        })

        propertySheet.addProperty({
            key: "Sharpen",
            type: PropertyType.BUTTON,
            config: {
                action: this.sharpenSegment.bind(this)
            }
        })
    }

    sharpenSegment() {
        this.curSegment.handleIn = 0
        this.curSegment.handleOut = 0

        // After morph, the position of the shape might be shifted, so we need to store the new position in the Cpp side.
        this.targetShape.store({position: true, segments: true})
    }

    smoothSegment() {
        this.curSegment.smooth()

        // After morph, the position of the shape might be shifted, so we need to store the new position in the Cpp side.
        this.targetShape.store({position: true, segments: true})
    }

    protected showInspector() {
        // Show inspector
        let propertySheet: PropertySheet = new PropertySheet()
        this.setupPropertySheet(propertySheet)
        IDEEventBus.getInstance().emit(EventNames.OBJECTSELECTED, propertySheet, this.curSegment)
    }

    dragging(pos) {
        if (this.isDragging && this.curSegment != null && !this.targetShape.isLocked()) {
            let startPoint = new paper.Point(this.startPos.x, this.startPos.y)
            let posPoint = new paper.Point(pos.x, pos.y)
            let localStart = this.targetShape.globalToLocal(startPoint)
            let localPos = this.targetShape.globalToLocal(posPoint)
            let offset = localPos.subtract(localStart)

            if (this.pressingShift) {
                if (Math.abs(offset.x) > Math.abs(offset.y)) {
                    offset.y = 0.0
                } else {
                    offset.x = 0.0
                }
            }

            let proposedNewPosition = this.curSegmentStartPos.add(offset)

            let shapeSegmentMoveCommand = new ShapeSegmentMoveCommand(this.targetShape, this.curSegment.index, this.curSegmentStartPos, proposedNewPosition)
            shapeSegmentMoveCommand.DoCommand()
            undoManager.PushCommand(shapeSegmentMoveCommand)

            this.valueChangeHandler.callHandlers("point", this.curSegment)
        }
    }
}

class ShapeHandlerMoveHandler extends ShapeMorphHandler {
    targetHandleName: string = ""
    targetHandleStartPos: paper.Point = null

    valueChangeHandlerMap: Map<string, Function> = new Map<string, Function>()

    beginMove(startPos, hitResult, showInspector:boolean = true) {
        super.beginMove(startPos, hitResult, showInspector);
        if(hitResult == null)
            return

        if (hitResult.type == "handle-in")
            this.targetHandleName = "handleIn"
        else
            this.targetHandleName = "handleOut"

        this.targetHandleStartPos = this.curSegment[this.targetHandleName].clone()
    }

    dragging(pos) {
        if (this.isDragging && this.curSegment != null && !this.targetShape.isLocked()) {
            let startPoint = new paper.Point(this.startPos.x, this.startPos.y)
            let posPoint = new paper.Point(pos.x, pos.y)
            let localStart = this.targetShape.globalToLocal(startPoint)
            let localPos = this.targetShape.globalToLocal(posPoint)
            let offset = localPos.subtract(localStart)

            if (this.pressingShift) {
                if (Math.abs(offset.x) > Math.abs(offset.y)) {
                    offset.y = 0.0
                } else {
                    offset.x = 0.0
                }
            }

            let targetHandlePos = this.targetHandleStartPos.add(offset)

            let shapeSegmentMoveCommand = new ShapeHandlerMoveCommand(this.targetShape, this.curSegment.index, this.targetHandleName, this.targetHandleStartPos, targetHandlePos)
            shapeSegmentMoveCommand.DoCommand()
            undoManager.PushCommand(shapeSegmentMoveCommand)

            if (this.valueChangeHandlerMap.get(this.targetHandleName)) {
                this.valueChangeHandlerMap.get(this.targetHandleName)(this.curSegment[this.targetHandleName])
            }
        }
    }
}

class ShapeInsertSegmentHandler extends ShapeMorphHandler {
    beginMove(startPos, hitResult = null, showInspector: boolean = true) {
        super.beginMove(startPos);
        if(this.targetShape.isLocked())
            return

        if (!this.targetShape.isSegmentSeletable())
            return

        let shapeSegmentInsertCommand = new ShapeSegmentInsertCommand(this.targetShape, startPos)
        shapeSegmentInsertCommand.DoCommand()
        this.setSegment(shapeSegmentInsertCommand.segment)

        undoManager.PushCommand(shapeSegmentInsertCommand)

        if(showInspector)
            this.showInspector()

        this.valueChangeHandler.callHandlers("insertSegment", this.curSegment)
    }
}

let shapeHandlerMoveHandler = new ShapeHandlerMoveHandler()
let shapeMorphHandler = new ShapeMorphHandler()
let shapeInsertSegmentHandler = new ShapeInsertSegmentHandler()
export {shapeMorphHandler, shapeHandlerMoveHandler, shapeInsertSegmentHandler,
    ShapeMorphHandler, ShapeInsertSegmentHandler, ShapeHandlerMoveHandler}