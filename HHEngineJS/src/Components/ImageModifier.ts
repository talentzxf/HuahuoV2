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

    afterUpdate() {
        if (this.imageLeft == 0 && this.imageRight == 0 && this.imageTop == 0 && this.imageBottom == 0) {
            return
        } else {
            let imageShape = this.baseShape as ImageShapeJS
            // @ts-ignore
            let originalRaster = imageShape.getOriginalRaster()

            let originalWidth = originalRaster.width
            let originalHeight = originalRaster.height

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
            imageRaster.setImageData(originalRaster.getImageData(imgRect), zeroPoint)
        }
        // this.modifiedDims.top = this.firstFrameDims.height / 2 * this.margins[0] / 100.0
        // this.modifiedDims.left = this.firstFrameDims.width / 2 * this.margins[1] / 100.0
        //
        // let remainWidth = this.firstFrameDims.width - this.modifiedDims.left
        // let rightMargin = this.firstFrameDims.width / 2 * this.margins[2] / 100.0
        // this.modifiedDims.width = remainWidth - rightMargin
        //
        // let remainHeight = this.firstFrameDims.height - this.modifiedDims.top
        // let bottomMargin = this.firstFrameDims.height / 2 * this.margins[3] / 100.0
        // this.modifiedDims.height = remainHeight - bottomMargin
        //
        // let newShape = this.originallyShape.getSubRaster(new paper.Rectangle(this.modifiedDims.left, this.modifiedDims.top,
        //     this.modifiedDims.width, this.modifiedDims.height))
        // newShape.data.meta = this
    }
}

export {ImageModifier}