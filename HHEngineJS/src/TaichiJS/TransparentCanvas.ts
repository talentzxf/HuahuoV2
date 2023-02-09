import {SetTransparentCanvasImage} from "./SetTransparentCanvasImage";

class TransparentCanvas {
    constructor(public htmlCanvas: HTMLCanvasElement) {
        this.setImageObj = new SetTransparentCanvasImage(htmlCanvas)
    }
    private setImageObj: SetTransparentCanvasImage
    async setImage(image) {
        await this.setImageObj.render(image)
    }
}

export { TransparentCanvas }