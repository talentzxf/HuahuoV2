import {CreateShapeCommand} from "../RedoUndo/CreateShapeCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {elementCreator} from "./ElementCreator";
import {huahuoEngine, BaseShapeJS} from "hhenginejs";
import {Logger} from "hhcommoncomponents";
import {LayerGraphWrapper} from "./LayerGraphWrapper";

class LayerUtils {
    static addShapeToCurrentLayer(shape: BaseShapeJS) {
        let currentLayer = huahuoEngine.GetCurrentLayer()

        shape.saveAsKeyFrame();

        let createShapeCommand = new CreateShapeCommand(currentLayer, shape)
        createShapeCommand.DoCommand()

        undoManager.PushCommand(createShapeCommand)

        elementCreator.dispatchElementChange(shape.getBornStoreId())
    }

    static InitLayer(layer) {
        if (!layer.inited) {

            layer.addShape = (shape) => {
                shape.update()
                layer.AddShapeInternal(shape.getRawObject())
                shape.isPermanent = true
                shape.isDeleted = false

                if (huahuoEngine.getActivePlayer()) {
                    huahuoEngine.getActivePlayer().getLayerShapes(layer).set(shape.getRawObject().ptr, shape)
                }

                huahuoEngine.hasShape = true

                Logger.debug("Currently there're:" + layer.GetShapeCount() + " shapes in the layer.")
            }

            layer.getGraphObject = (frameId, createIfNoExisted = false) => {
                if (layer.hasOwnProperty("graphObject"))
                    return layer.graphObject

                let eventGraphParam = layer.GetFrameEventGraphParam(frameId, createIfNoExisted)
                if (eventGraphParam != null) {
                    layer.graphObject = new LayerGraphWrapper(eventGraphParam)

                    return layer.graphObject
                }

                return null
            }

            layer.inited = true
        }
    }
}


export {LayerUtils}