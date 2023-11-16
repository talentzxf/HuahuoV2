import {PropertyType, dataURItoBlob} from "hhcommoncomponents"
import {parseGIF, decompressFrames, ParsedFrame} from "gifuct-js";
import {GlobalConfig} from "../GlobalConfig"
import {AbstractMediaShapeJS} from "./AbstractMediaShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";
import {huahuoEngine} from "../EngineAPI";
import {ImageModifier} from "../Components/ImageModifier";
import Raster = paper.Raster;

let shapeName = "ImageShape"

class ImageShapeJS extends AbstractMediaShapeJS {

    static createImageShape(rawObj) {
        return new ImageShapeJS(rawObj);
    }

    // Animation related properties
    frames: ParsedFrame[] = null;
    animationLoaded: boolean = false;
    worldFrameAnimationFrameMap: Map<number, number> = null;
    lastAnimationFrame: number = -1; // The animation frame used in last time update.
    animationTotalWorldFrames: number = -1;

    resourceMD5: string

    firstFrameDims = new paper.Rectangle(-1, -1, -1, -1)

    set isAnimation(isAnimation: boolean) {
        this.rawObj.SetIsAnimation(isAnimation)
    }

    get isAnimation() {
        return this.rawObj.GetIsAnimation()
    }

    getShapeName(): string {
        return shapeName
    }

    onDataLoaded() {
        this.isAnimation = this.rawObj.GetIsAnimation()
        if (this.isAnimation)
            this.loadFrameData()

        if (this.paperItem) {
            let raster = this.paperItem as paper.Raster
            raster.onLoad = () => {
                this.firstFrameDims.width = raster.width
                this.firstFrameDims.height = raster.height

                // Force update, so some component logic can be executed.
                this.update(true)
            }

            raster.source = this.data
        }
    }

    loadFrameData() {
        if (!this.animationLoaded) {
            let binaryData: Uint8Array = dataURItoBlob(this.data)
            let gif = parseGIF(binaryData)
            this.frames = decompressFrames(gif, true)

            // If the disposalType == 2, we need to draw it on top of the previous frame.
            // That means, we have to draw the gif from beginning to end here to get the real frame data.
            let tempCanvas = document.createElement("canvas")
            let tempCtx = tempCanvas.getContext("2d")

            let gifCanvas = document.createElement("canvas")
            let gifCtx = gifCanvas.getContext("2d", {willReadFrequently: true})

            let frameImageData = null

            let elapsedTime = 0.0
            this.worldFrameAnimationFrameMap = new Map<number, number>()
            let animationFrameId = 0
            let lastWorldFrame = -1

            this.firstFrameDims = new paper.Rectangle(this.frames[0].dims)
            gifCanvas.width = this.firstFrameDims.width
            gifCanvas.height = this.firstFrameDims.height

            let needDisposal = false
            for (let frame of this.frames) {
                let dims = frame.dims
                if (!frameImageData || dims.width != frameImageData.width || dims.height != frameImageData.height) {
                    tempCanvas.width = dims.width
                    tempCanvas.height = dims.height
                    frameImageData = tempCtx.createImageData(dims.width, dims.height)
                }

                if (needDisposal) {
                    gifCtx.clearRect(0, 0, tempCanvas.width, tempCanvas.height)
                    needDisposal = false
                }

                frameImageData.data.set(frame.patch)
                tempCtx.putImageData(frameImageData, 0, 0)
                gifCtx.drawImage(tempCanvas, dims.left, dims.top)

                frame["realImageData"] = gifCtx.getImageData(0, 0, gifCanvas.width, gifCanvas.height).data

                if (frame.disposalType === 2) {
                    needDisposal = true
                }

                let elapsedFrames = Math.floor(elapsedTime * GlobalConfig.fps);

                for (let frameId = lastWorldFrame + 1; frameId <= elapsedFrames; frameId++) {
                    this.worldFrameAnimationFrameMap.set(frameId, animationFrameId)
                    lastWorldFrame = frameId
                }

                let delay = frame.delay ? frame.delay / 1000.0 : 1.0 / GlobalConfig.fps

                elapsedTime += delay // delay is in milliseconds
                animationFrameId++
            }

            this.animationTotalWorldFrames = lastWorldFrame

            this.animationLoaded = true

            let maxFrameId = this.bornFrameId + this.worldFrameAnimationFrameMap.size
            // Update the max frame of the layer.
            let store = huahuoEngine.GetStoreById(this.getBornStoreId())
            store.UpdateMaxFrameId(maxFrameId)
        }
    }

    createShape() {
        super.createShape()

        let _paper: any = this.getPaperJs()
        let tempShape = new _paper.Raster()
        tempShape.data.meta = this
        tempShape.position = _paper.view.center

        tempShape.onLoad = () => {
            this.firstFrameDims.width = tempShape.width
            this.firstFrameDims.height = tempShape.height
        }

        tempShape.source = this.data // If it's gif, the first frame will be showed here.
        tempShape.fillColor = new _paper.Color("red")

        // As data is loaded asynchronously, it might not be ready when the shape is created.
        // In that case, should load the frame data when the load is completed.
        if (this.isAnimation) {
            if (this.data) {
                this.loadFrameData()
            } else {
                let _this = this
                this.loadDataFromCpp().then(() => {
                    _this.loadFrameData()
                })
            }
        }

        this.paperItem = tempShape

        this.appendProperties()

        super.afterCreateShape()
    }

    appendProperties() {
        super.appendProperties()

        if (this.isAnimation) {
            // Position
            this.propertySheet.addProperty({
                key: "inspector.image.Frames",
                type: PropertyType.STRING,
                getter: this.getFrames.bind(this)
            });

        }
    }

    private originalCanvasCtx: CanvasRenderingContext2D = null

    private _getOriginalImageCtx(){
        if(this.originalCanvasCtx == null){
            let canvas:HTMLCanvasElement = document.createElement("canvas")
            canvas.width = this.firstFrameDims.width
            canvas.height = this.firstFrameDims.height
            this.originalCanvasCtx = canvas.getContext("2d")
        }

        return this.originalCanvasCtx
    }

    public getOriginaglImageCtx(): CanvasRenderingContext2D { // Call this only from the ImageModifier. Or else it might be changed!
        // If this is animation. It will be updated every frame. So it's the originalFrame
        if (this.isAnimation || this.originalCanvasCtx == null) {
            // @ts-ignore
            this._getOriginalImageCtx().putImageData((this.paperItem as Raster).getImageData(), 0, 0)
        }

        return this.originalCanvasCtx
    }

    afterUpdate(force: boolean = false) {
        if (!this.isLoaded()) {
            return
        }
        super.afterUpdate(force);

        if (this.rawObj.IsVisible() && this.isAnimation) {
            let bornFrameId = this.rawObj.GetBornFrameId();
            let worldFrameId = this.getLayer().GetCurrentFrame();

            let curFrameId = Math.floor((worldFrameId - bornFrameId) % this.animationTotalWorldFrames)
            let playingAnimationFrameId = this.worldFrameAnimationFrameMap.get(curFrameId)

            if (this.lastAnimationFrame != playingAnimationFrameId) {
                let frame = this.frames[playingAnimationFrameId]
                let raster = this.paperItem as paper.Raster
                raster.clear()

                let dims = this.firstFrameDims
                let frameImageData = raster.createImageData(new paper.Size(dims.width, dims.height))
                frameImageData.data.set(frame["realImageData"])
                raster.setImageData(frameImageData, new paper.Point(dims.left, dims.top))

                this.lastAnimationFrame = playingAnimationFrameId
            }
        }
    }

    getFrames() {
        return this.worldFrameAnimationFrameMap.size;
    }

    initShapeFromEditor() {
        super.initShapeFromEditor();

        this.addComponent(new ImageModifier())
    }
}

clzObjectFactory.RegisterClass(shapeName, ImageShapeJS.createImageShape)

export {ImageShapeJS}