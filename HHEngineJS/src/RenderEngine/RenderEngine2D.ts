import {Vector2} from "hhcommoncomponents"
import {CameraBox} from "./CameraBox";

interface RenderEngine2D {
    init(canvas): void;

    clearBackground(): void;

    getWorldPosFromView(x: number, y: number): Vector2

    zoomIn(scale: number): void

    zoomOut(scale: number): void

    zoomReset(): void

    resize(canvas: HTMLCanvasElement, width: number, height: number): void

    getContentWH(canvasWidth, canvasHeight): [number, number];

    setBgColor(bgColor);

    getAspectRatio()

    focusShape(shape)

    setViewPosition(position)

    createViewRectangle()

    getCameraBox(): CameraBox
}

export {RenderEngine2D}