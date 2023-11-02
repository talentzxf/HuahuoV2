import {RenderEngine2D} from "./RenderEngine2D";

class CameraBox {
    _rectangle: paper.Path.Rectangle
    _renderEngine: RenderEngine2D

    // 0.0 -- No margin at all. Viewport's position the same as the focus target.
    // 1.0 -- Viewport will go with the
    margin: 0.0

    constructor(renderEngine) {
        this._renderEngine = renderEngine
    }

    private get rectangle() {
        if (this._rectangle == null) {
            this._rectangle = this._renderEngine.createViewRectangle()
            this._rectangle.strokeColor = new paper.Color("black")

            this._rectangle.scaling = new paper.Point(this.margin, this.margin)
        }

        return this._rectangle
    }

    show() {
        this.rectangle.visible = true
    }

    hide() {
        this.rectangle.visible = false
    }
}

export {CameraBox}