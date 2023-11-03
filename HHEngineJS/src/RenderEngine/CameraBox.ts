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
            this._rectangle = this._renderEngine.createViewRectangle(null)
            this._rectangle.applyMatrix = false
            this._rectangle.strokeColor = new paper.Color("red")
            this._rectangle.strokeWidth = 10;
            this._rectangle.dashArray = [10, 12]

            if (this.margin > 0.0) {
                this._rectangle.scaling = new paper.Point(this.margin, this.margin)
            }

            this._rectangle.bringToFront()
        }

        return this._rectangle
    }

    getViewCenterPosition(focusPoint) {
        let currentRectangle = this.rectangle.bounds
        if (currentRectangle.contains(focusPoint)) {
            return currentRectangle.center
        }

        let currentCenter = currentRectangle.center
        let deltaX = 0
        let deltaY = 0
        if (focusPoint.x < currentRectangle.left) {
            deltaX = focusPoint.x - currentRectangle.left
        }

        if (focusPoint.x > currentRectangle.right) {
            deltaX =  focusPoint.x - currentRectangle.right
        }

        if (focusPoint.y < currentRectangle.top) {
            deltaY = focusPoint.y - currentRectangle.top
        }

        if (focusPoint.y > currentRectangle.bottom) {
            deltaY = focusPoint.y - currentRectangle.bottom
        }

        currentCenter.x += deltaX
        currentCenter.y += deltaY

        return currentCenter
    }

    setMargin(margin) {
        this.margin = margin

        // Not sure why, but sometimes, if we change scaling, the position will also be changed.
        let oldPosition = this.rectangle.position
        this.rectangle.scaling = new paper.Point(this.margin, this.margin)
        this.rectangle.position = oldPosition
    }

    show() {
        this.rectangle.visible = true
    }

    hide() {
        this.rectangle.visible = false
    }

    reset(viewCenter) {
        this.rectangle.position = viewCenter
    }
}

export {CameraBox}