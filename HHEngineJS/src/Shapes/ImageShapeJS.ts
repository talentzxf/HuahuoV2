import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import {PropertyType, getMimeTypeFromDataURI, dataURItoBlob} from "hhcommoncomponents"
import {parseGIF, decompressFrames, ParsedFrame} from "gifuct-js";
import {GlobalConfig} from "../GlobalConfig"
import {AbstractMediaShapeJS} from "./AbstractMediaShapeJS";

let shapeName = "ImageShape"
class ImageShapeJS extends AbstractMediaShapeJS{

    static createImageShape(rawObj){
        return new ImageShapeJS(rawObj);
    }

    // Animation related properties
    isAnimation: boolean = false
    frames: ParsedFrame[] = null;
    animationLoaded: boolean = false;
    worldFrameAnimationFrameMap: Map<number, number> = null;
    lastAnimationFrame: number = -1; // The animation frame used in last time update.
    animationTotalWorldFrames: number = -1;

    setData(fName, data, isAnimation = false){
        super.setData(fName, data)
        this.isAnimation = isAnimation
    }

    getShapeName(): string {
        return shapeName
    }

    onDataLoaded() {
        this.isAnimation = this.rawObj.GetIsAnimation()
        if(this.isAnimation)
            this.loadFrameData()

        if(this.paperItem){
            let raster = this.paperItem as paper.Raster
            raster.source = this.data
        }
    }

    loadFrameData(){
        if(!this.animationLoaded){
            let binaryData:Uint8Array = dataURItoBlob(this.data)
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

                let delay = frame.delay?frame.delay/1000.0: 1.0/GlobalConfig.fps

                elapsedTime += delay // delay is in milliseconds
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
        tempShape.source = this.data // If it's gif, the first frame will be showed here.

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

    beforeUpdate() {
        super.beforeUpdate();

        if(!this.loaded){
            return;
        }

        if(this.rawObj.IsVisible() && this.isAnimation){
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
        this.rawObj.SetIsAnimation(this.isAnimation)
    }

    getFrames(){
        return this.worldFrameAnimationFrameMap.size;
    }
}

shapeFactory.RegisterClass(shapeName, ImageShapeJS.createImageShape)

export {ImageShapeJS}