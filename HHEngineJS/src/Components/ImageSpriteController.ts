import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {ImageShapeJS} from "../Shapes/ImageShapeJS";
import {layerUtils} from "../LayerUtils";
import {GetterProperty} from "hhcommoncomponents";

@Component({compatibleShapes: ["ImageShapeJS"], maxCount: 1})
class ImageSpriteController extends AbstractComponent {
    @PropertyValue(PropertyCategory.interpolateFloat, 1.0)
    rowCount
    @PropertyValue(PropertyCategory.interpolateFloat, 1.0)
    colCount
    @PropertyValue(PropertyCategory.interpolateFloat, 1.0)
    playSpeed

    @PropertyValue(PropertyCategory.interpolateFloat, null, {observedFields: ["rowCount", "colCount", "playSpeed"]} as GetterProperty)
    getTotalFrameCount() {
        return Math.floor(this.rowCount * this.colCount / this.playSpeed)
    }

    lastFrameId = -1

    getCurrentFrameId() {
        return this.baseShape.getLayer().GetCurrentFrame()
    }

    needUpdate() {
        // The image is not ready yet.
        // @ts-ignore
        if (!this.baseShape.paperShape.loaded) {
            return false;
        }

        if (this.lastFrameId < 0) {
            return true
        }

        if (this.lastFrameId != this.getCurrentFrameId()) {
            return true
        }

        return false
    }

    calculateLocalFrame() {
        let currentFrameId = this.getCurrentFrameId()
        let bornFrame = this.baseShape.bornFrameId
        let totalFrameCount = this.rowCount * this.colCount

        return layerUtils.uniformFrameId((currentFrameId - bornFrame) * this.playSpeed, totalFrameCount)
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (this.rowCount <= 0 || this.colCount <= 0) {
            return;
        }

        if (this.needUpdate()) {
            let imageShape = this.baseShape as ImageShapeJS
            // @ts-ignore
            let originalImageCtx = imageShape.getOriginaglImageCtx()

            let originalWidth = originalImageCtx.canvas.width
            let originalHeight = originalImageCtx.canvas.height

            let spriteWidth = originalWidth / this.colCount
            let spriteHeight = originalHeight / this.rowCount

            // @ts-ignore
            imageShape.paperItem.width = spriteWidth
            // @ts-ignore
            imageShape.paperItem.height = spriteHeight

            let localFrameId = this.calculateLocalFrame()
            let coordY = Math.floor(localFrameId / this.colCount)
            let coordX = localFrameId % this.colCount

            let spriteLeft = coordX * spriteWidth
            let spriteTop = coordY * spriteHeight

            // @ts-ignore
            let imageRaster = imageShape.paperItem as paper.Raster
            let imgRect = new paper.Rectangle(spriteLeft, spriteTop, spriteWidth, spriteHeight)
            imageRaster.clear()
            // @ts-ignore
            imageRaster.setImageData(originalImageCtx.getImageData(imgRect.left, imgRect.top, imgRect.width, imgRect.height), new paper.Point(0, 0))
        }
    }
}

export {ImageSpriteController}