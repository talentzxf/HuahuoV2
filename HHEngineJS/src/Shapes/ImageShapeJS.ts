import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"
import {parseGIF, decompressFrames, ParsedFrame} from "gifuct-js";
import {GlobalConfig} from "../GlobalConfig"

function getMimeType(dataURI): string{
    // separate out the mime component
    let mimeString = dataURI.split(',')[0].split(':')[1].split(';')[0]
    return mimeString
}

function dataURItoBlob(dataURI): Uint8Array{
    // convert base64 to raw binary data held in a string
    // doesn't handle URLEncoded DataURIs - see SO answer #6850276 for code that does this
    let byteString = atob(dataURI.split(',')[1]);

    // write the bytes of the string to an ArrayBuffer
    let ab = new ArrayBuffer(byteString.length);

    // create a view into the buffer
    let ia = new Uint8Array(ab);

    // set the bytes of the buffer to the correct values
    for (let i = 0; i < byteString.length; i++) {
        ia[i] = byteString.charCodeAt(i);
    }
    return ia;
}

let shapeName = "ImageShape"
class ImageShapeJS extends BaseShapeJS{

    static createImageShape(rawObj){
        return new ImageShapeJS(rawObj);
    }

    imgData:string = null
    fileName: string = ""

    loaded: boolean = false

    dirty: boolean = false

    // Animation related properties
    isAnimation: boolean = false
    frames: ParsedFrame[] = null;
    animationLoaded: boolean = false;
    worldFrameAnimationFrameMap: Map<number, number> = null;
    lastAnimationFrame: number = -1; // The animation frame used in last time update.
    animationTotalWorldFrames: number = -1;

    setImageData(fName, data, isAnimation = false){
        this.fileName = fName
        this.imgData = data
        this.dirty = true
        this.isAnimation = isAnimation
    }

    getShapeName(): string {
        return shapeName
    }

    awakeFromLoad() {
        let dataLength = this.rawObj.GetImageDataSize()

        // write the bytes of the string to an ArrayBuffer
        let ab = new ArrayBuffer(dataLength);

        let binaryData:Uint8Array = new Uint8Array(ab)

        for(let idx = 0; idx < dataLength; idx++){
            binaryData[idx] = this.rawObj.GetImageDataAtIndex(idx);
        }
        // this.rawObj.LoadImageData(binaryData)
        // console.log("Loaded")

        let mimeType:string = this.rawObj.GetImageMimeType()
        let blob = new Blob([binaryData], {'type': mimeType})

        let reader = new FileReader()
        reader.readAsDataURL(blob)

        let _this = this
        reader.onload = function(){
            _this.imgData = reader.result as string

            _this.isAnimation = _this.rawObj.GetIsAnimation()
            if(_this.isAnimation)
                _this.loadFrameData()

            if(_this.paperItem){
                let raster = _this.paperItem as paper.Raster
                raster.source = _this.imgData
            }

            _this.loaded = true
        }


        super.awakeFromLoad();
    }

    loadFrameData(){
        if(!this.animationLoaded){
            let binaryData:Uint8Array = dataURItoBlob(this.imgData)
            let gif = parseGIF(binaryData)
            this.frames = decompressFrames(gif, true)

            let elapsedTime = 0.0
            this.worldFrameAnimationFrameMap = new Map<number, number>()
            let animationFrameId = 0
            let lastWorldFrame = -1
            for(let frame of this.frames){
                let elapsedFrames = Math.floor(elapsedTime * GlobalConfig.fps);

                for(let frameId = lastWorldFrame + 1; frameId <= elapsedFrames; frameId++){
                    this.worldFrameAnimationFrameMap.set(frameId, animationFrameId)
                    lastWorldFrame = frameId
                }

                elapsedTime += frame.delay/1000.0 // delay is in milliseconds
                animationFrameId++
            }

            this.animationTotalWorldFrames = lastWorldFrame

            this.animationLoaded = true
        }
    }

    createShape(){
        let _this = this
        let _paper:any = this.getPaperJs()
        let tempShape = new _paper.Raster()
        tempShape.data.meta = this
        tempShape.position = _paper.view.center
        tempShape.source = this.imgData // If it's gif, the first frame will be showed here.

        tempShape.onLoad = function(){
            _this.loaded = true

            _this.dirty = false
        }

        if(this.isAnimation){
            this.loadFrameData()
        }

        this.paperItem = tempShape

        this.appendProperties()
    }

    appendProperties(){
        if(this.isAnimation){
            // Position
            this.propertySheet.addProperty({
                key: "Frames",
                type: PropertyType.STRING,
                getter: this.getFrames.bind(this)
            });

        }
    }

    beforeUpdate(updateOptions) {
        super.beforeUpdate(updateOptions);

        if(!this.loaded){
            return;
        }

        if(this.rawObj.IsVisible()){
            let bornFrameId = this.rawObj.GetBornFrameId();
            let worldFrameId = this.getLayer().GetCurrentFrame();

            let curFrameId = Math.floor((worldFrameId - bornFrameId) % this.animationTotalWorldFrames)
            let playingAnimationFrameId = this.worldFrameAnimationFrameMap.get(curFrameId)

            if(this.lastAnimationFrame != playingAnimationFrameId){
                let frame = this.frames[playingAnimationFrameId]
                let raster = this.paperItem as paper.Raster
                raster.clear()

                let dims = frame.dims
                let frameImageData = raster.createImageData(new paper.Size(dims.width, dims.height))
                frameImageData.data.set(frame.patch)
                raster.setImageData(frameImageData, new paper.Point(0,0))

                this.lastAnimationFrame = playingAnimationFrameId
            }
        }
    }

    store() {
        super.store();

        if(!this.imgData.startsWith("blob") && this.dirty){
            let binaryData:Uint8Array = dataURItoBlob(this.imgData)
            this.rawObj.SetImageData(binaryData, binaryData.length);
            this.rawObj.SetImageMimeType(getMimeType(this.imgData))
            this.rawObj.SetIsAnimation(this.isAnimation)

            this.dirty = false
        }
    }

    getFrames(){
        return this.frames.length;
    }
}

shapeFactory.RegisterClass(shapeName, ImageShapeJS.createImageShape)

export {ImageShapeJS}