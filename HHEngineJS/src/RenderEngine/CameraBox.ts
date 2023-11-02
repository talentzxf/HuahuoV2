import {RenderEngine2D} from "./RenderEngine2D";
import * as paper from "paper"

class CameraBox {
    _rectangle: paper.Path.Rectangle
    _renderEngine: RenderEngine2D

    // 0.0 -- No margin at all. Viewport's position the same as the focus target.
    // 1.0 -- Viewport will go with the
    margin: 0.5

    constructor(renderEngine) {
        this._renderEngine = renderEngine
    }

    private get rectangle() {
        if (this._rectangle == null) {
            this._rectangle = this._renderEngine.createViewRectangle(new paper.Color("red"))
            this._rectangle.applyMatrix = false

            if (this.margin > 0.0) {
                this._rectangle.scaling = new paper.Point(this.margin, this.margin)
            }

            this._rectangle.bringToFront()
        }

        return this._rectangle
    }

    setMargin(margin) {
        this.margin = margin

        this.rectangle.scaling = new paper.Point(this.margin, this.margin)
    }

    show() {
        this.rectangle.visible = true
    }

    hide() {
        this.rectangle.visible = false
    }
}

export {CameraBox}