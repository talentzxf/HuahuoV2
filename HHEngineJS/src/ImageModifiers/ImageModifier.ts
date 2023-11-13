class Dims {
    width: number;
    height: number;
    top: number;
    left: number
}

class ImageFrame {
    dims: Dims
    data: string
}

abstract class ImageModifier {
    abstract modifyImage(imgFrame: ImageFrame): ImageFrame
}

class ImageModifierStack extends ImageModifier {
    modifierStack: Array<ImageModifier> = new Array<ImageModifier>()

    modifyImage(imgFrame: ImageFrame): ImageFrame {
        let modifiedFrame = imgFrame
        for (let modifier of this.modifierStack) {
            modifiedFrame = modifier.modifyImage(modifiedFrame)
        }

        return modifiedFrame
    }
}

export {Dims, ImageModifierStack, ImageModifier, ImageFrame}