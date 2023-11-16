import {CreateShapeCommand} from "../RedoUndo/CreateShapeCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {elementCreator} from "./ElementCreator";
import {BaseShapeJS, huahuoEngine} from "hhenginejs";
import {PropertySheet, PropertyType} from "hhcommoncomponents";
import {formManager} from "../Utilities/FormManager";
import {EventGraphForm} from "../EventGraphUI/EventGraphForm";
import {sceneViewManager} from "./SceneViewManager";

function openFrameEventGraphForm() {
    let eventGraphForm = formManager.openForm(EventGraphForm)

    let currentLayer = huahuoEngine.GetCurrentLayer()
    let frameId = currentLayer.GetCurrentFrame()

    let frameEventGraphWrapperObject = huahuoEngine.getWrappedGraphObjectForLayer(currentLayer, frameId, true)
    eventGraphForm.setTargetComponent(frameEventGraphWrapperObject)

    huahuoEngine.getFocusedSceneView().timeline.redrawCanvas()
}

class EditorLayerUtils {
    static addShapeToCurrentLayer(shape: BaseShapeJS) {
        let currentLayer = huahuoEngine.GetCurrentLayer()

        shape.saveAsKeyFrame();

        let createShapeCommand = new CreateShapeCommand(currentLayer, shape)
        createShapeCommand.DoCommand()

        undoManager.PushCommand(createShapeCommand)

        elementCreator.dispatchElementChange(shape.getBornStoreId())
    }

    static buildLayerCellProperties(layer, frameId) {
        let property = {
            key: "inspector.layerCellProperty",
            type: PropertyType.COMPONENT,
            config: {
                children: []
            }
        }

        property.config.children.push({
            key: "inspector.LayerName",
            type: PropertyType.STRING,
            getter: layer.GetName.bind(layer)
        })

        property.config.children.push({
            key: "inspector.frameId",
            type: PropertyType.NUMBER,
            getter: () => {
                return frameId + 1
            }
        })

        property.config.children.push({
            key: "inspector.editFrameEventGraph",
            type: PropertyType.BUTTON,
            config: {
                action: openFrameEventGraphForm
            }
        })

        property.config.children.push({
            key: () => {
                if (layer.IsStopFrame(frameId))
                    return "inspector.unsetAsStopFrame"
                return "inspector.setAsStopFrame"
            },
            type: PropertyType.BUTTON,
            config: {
                action: () => {
                    if (layer.IsStopFrame(frameId)) {
                        layer.RemoveStopFrame(frameId)
                    } else {
                        layer.AddStopFrame(frameId)
                    }

                    sceneViewManager.getFocusedSceneView().timeline.redrawCanvas()
                }
            }
        })

        let propertySheet = new PropertySheet()
        propertySheet.addProperty(property)

        return propertySheet
    }
}


export {EditorLayerUtils}