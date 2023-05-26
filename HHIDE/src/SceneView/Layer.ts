import {CreateShapeCommand} from "../RedoUndo/CreateShapeCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {elementCreator} from "./ElementCreator";
import {huahuoEngine, BaseShapeJS} from "hhenginejs";

class LayerUtils{
    static addShapeToCurrentLayer(shape:BaseShapeJS){
        let currentLayer = huahuoEngine.GetCurrentLayer()

        shape.saveAsKeyFrame();

        let createShapeCommand = new CreateShapeCommand(currentLayer, shape)
        createShapeCommand.DoCommand()

        undoManager.PushCommand(createShapeCommand)

        elementCreator.dispatchElementChange(shape.getBornStoreId())
    }
}


export {LayerUtils}