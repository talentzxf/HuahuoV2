import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import {FloatPropertyConfig} from "hhcommoncomponents/dist/src/Properties/PropertyConfig";
import {ImageShapeJS} from "../Shapes/ImageShapeJS";

@Component({compatibleShapes: ["ImageShapeJS"], maxCount: 1})
class ImageModifier extends AbstractComponent {
    @PropertyValue(PropertyCategory.interpolateFloat, 0.0, {min: 0.0, max: 100.0, step: 1.0} as FloatPropertyConfig)
    imageTop
    @PropertyValue(PropertyCategory.interpolateFloat, 0.0, {min: 0.0, max: 100.0, step: 1.0} as FloatPropertyConfig)
    imageLeft
    @PropertyValue(PropertyCategory.interpolateFloat, 0.0, {min: 0.0, max: 100.0, step: 1.0} as FloatPropertyConfig)
    imageRight
    @PropertyValue(PropertyCategory.interpolateFloat, 0.0, {min: 0.0, max: 100.0, step: 1.0} as FloatPropertyConfig)
    imageBottom

    prevImageRect = null

    dimChanged() {
        if ((this.baseShape as ImageShapeJS).isAnimation)
            return true
        if (this.prevImageRect == null)
            return true
        if (this.prevImageRect.top == this.imageTop && this.prevImageRect.bottom == this.imageBottom
            && this.prevImageRect.left == this.imageLeft && this.prevImageRect.right == this.imageRight) {
            return false
        }
        return true
    }

    afterUpdate() {
        if (this.imageLeft == 0 && this.imageRight == 0 && this.imageTop == 0 && this.imageBottom == 0) {
            return
        } else {

            if (this.dimChanged()) {
                let imageShape = this.baseShape as ImageShapeJS
                // @ts-ignore
                let originalImageCtx = imageShape.getOriginaglImageCtx()

                let originalWidth = originalImageCtx.canvas.width
                let originalHeight = originalImageCtx.canvas.height

                let top = originalHeight / 2 * this.imageTop / 100.0
                let left = originalWidth / 2 * this.imageLeft / 100.0

                let remainWidth = originalWidth - left
                let rightMargin = originalWidth / 2 * this.imageRight / 100.0
                let resultWidth = remainWidth - rightMargin

                let remainHeight = originalHeight - top
                let bottomMargin = originalHeight / 2 * this.imageBottom / 100.0
                let resultHeight = remainHeight - bottomMargin

                // Override the image content
                // @ts-ignore
                let imageRaster = imageShape.paperItem as paper.Raster
                let imgRect = new paper.Rectangle(left, top, resultWidth, resultHeight)
                let zeroPoint = new paper.Point(left, top)
                imageRaster.clear()
                imageRaster.setImageData(originalImageCtx.getImageData(imgRect.left, imgRect.top, imgRect.width, imgRect.height), zeroPoint)

                this.prevImageRect = imgRect
            }
        }
    }
}

export {ImageModifier}