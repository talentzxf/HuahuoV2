import * as React from "react"
import {PropertyProps} from "./BasePropertyX";
import {BaseShapeJS} from "hhenginejs";
import {CSSUtils} from "../Utilities/CSSUtils";
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {ShapePicker} from "../ShapeDrawers/ShapePicker";
import {HHToast} from "hhcommoncomponents";

type ShapePropertyState = {
    selectedObject: BaseShapeJS
}

class ShapePropertyX extends React.Component<PropertyProps, any> {
    state: ShapePropertyState = {
        selectedObject: null
    }

    shapePicker

    constructor(props) {
        super(props);

        this.shapePicker = new ShapePicker()
        this.shapePicker.onShapePicked = this.onShapePicked.bind(this)
    }

    onShapePicked(selectedShape: BaseShapeJS) {
        let property = this.props.property
        if (property.setter(selectedShape)) {
            this.onValueChanged(selectedShape)
        } else {
            HHToast.warn(i18n.t("toast.cantSelect"))
        }

        let currentFocusedSceneView = sceneViewManager.getFocusedSceneView()
        currentFocusedSceneView.resetDefaultShapeDrawer()
        currentFocusedSceneView.endOfDrawingShape(this.shapePicker)
    }

    onValueChanged(val) {
        this.state.selectedObject = val
        this.setState(this.state)
    }

    formatShapeName(type: string, name: string): string {
        if (type == null) {
            return "Unselected"
        }

        return i18n.t(type) + "[" + name + "]"
    }

    beginToPickupShape() {
        let currentFocusedSceneView = sceneViewManager.getFocusedSceneView()
        document.body.style.cursor = "pointer"
        currentFocusedSceneView.beginToDrawShape(this.shapePicker)
    }

    render() {
        let currentShape = this.props.property.getter()

        if(currentShape != this.state.selectedObject){
            this.state.selectedObject = currentShape
        }

        let targetShape = this.state.selectedObject

        let type = i18n.t("inspector.Unknown")
        let name = i18n.t("inspector.Unknown")

        if (targetShape != null) {
            type = i18n.t(targetShape.typename)
            name = targetShape.name
        }

        return (<div>
            <span>  {this.formatShapeName(type, name)} </span>
            <button className={"p-2 rounded " + CSSUtils.getButtonClass("indigo")}
                    onClick={this.beginToPickupShape.bind(this)}>
                {i18n.t("inspector.Pickup")}
            </button>
        </div>);
    }
}

export {ShapePropertyX}