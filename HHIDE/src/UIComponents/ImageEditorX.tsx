import * as React from "react";

type Dims = {
    width: number;
    height: number;
    top: number;
    left: number
}

type ImageFrame = {
    dims: Dims
    imageData: string
}

abstract class ImageModifier {
    abstract modifyImage(imageFrame: ImageFrame): ImageFrame
}

// Responsible chain.
class MarginImageModifier extends ImageModifier {
    dims: Dims

    getDims() {

    }

    modifyImage(imageFrame: ImageFrame): ImageFrame {
        return undefined;
    }

}

type ImageEditorState = {}

type ImageEditorProp = {
    data: ImageFrame // Original Image Frame
}

class ImageEditorX extends React.Component<any, ImageEditorState> {
    render() {
        return <>
            Render Image here here.
        </>;
    }
}