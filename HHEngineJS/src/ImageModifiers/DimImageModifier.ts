import {Dims, ImageFrame, ImageModifier} from "./ImageModifier";

class DimImageModifier extends ImageModifier {
    dims: Dims
    originalDims: Dims

    modifyImage(imgFrame: ImageFrame): ImageFrame {
        this.originalDims = imgFrame.dims
        imgFrame.dims = this.dims
        return imgFrame
    }
}