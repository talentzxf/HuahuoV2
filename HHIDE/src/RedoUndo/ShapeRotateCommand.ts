import {MergableCommand, UndoableCommand} from "./UndoManager";
import {BaseShapeJS} from "hhenginejs";

let commandName = "RotateShape"

class ShapeRotateCommand extends MergableCommand{
    targetShape:BaseShapeJS
    layer
    rotateDegree

    constructor(shape:BaseShapeJS, rotateDegree) {
        super()
        this.targetShape = shape
        this.layer = shape.getLayer()
        this.rotateDegree = rotateDegree
    }

    toString(){
        let cmdString = super.toString()

        let translatedTypeName = i18n.t(this.targetShape.getTypeName())

        cmdString += ":" + "[" + translatedTypeName + "]"
        if(this.targetShape.name)
            cmdString += this.targetShape.name

        return cmdString
    }

    _DoCommand() {
        this.targetShape.rotateAroundPivot(this.rotateDegree)
        this.targetShape.store()

        this.targetShape.update()
    }

    _UnDoCommand() {
        this.targetShape.rotateAroundPivot(-this.rotateDegree)
        this.targetShape.store()

        this.targetShape.update()
    }

    GetType(): string {
        return commandName;
    }

    MergeCommand(anotherCommand: MergableCommand): boolean { // Do we really need to merge the shape move command??
        // if(anotherCommand.GetType() == commandName){
        //     let moveCommand = anotherCommand as ShapeRotateCommand
        //     if(moveCommand){
        //         if(this.stackFrameEqual(anotherCommand)){
        //             // This is the same shape, in the same layer, in the same frameId, can merge the move command
        //             if(moveCommand.targetShape == this.targetShape &&
        //                 this.layer == moveCommand.layer){
        //                 this.rotateDegree = this.rotateDegree + moveCommand.rotateDegree
        //                 return true
        //             }
        //         }
        //     }
        // }

        return false
    }

}

export {ShapeRotateCommand}