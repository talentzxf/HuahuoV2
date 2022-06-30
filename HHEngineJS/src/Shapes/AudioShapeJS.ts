import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";
import {Property, PropertyType, getMimeTypeFromDataURI, dataURItoBlob} from "hhcommoncomponents"
import {GlobalConfig} from "../GlobalConfig";
import {AbstractMediaShapeJS} from "./AbstractMediaShapeJS";

let shapeName = "AudioShape"
class AudioShapeJS extends AbstractMediaShapeJS {
    static createAudioShape(rawObj) {
        return new AudioShapeJS(rawObj)
    }

    audioElement: HTMLAudioElement

    createShape() {
        let paperjs: any = this.getPaperJs()
        this.paperShape = new paperjs.Path.Circle(paperjs.view.center, 10);
        this.paperShape.fillColor = new paper.Color("cyan")
        this.paperShape.applyMatrix = false
        this.paperShape.data.meta = this

        this.appendProperties()
    }

    appendProperties(){
        let _this = this
        this.propertySheet.addProperty({
            key: "file name",
            type: PropertyType.STRING,
            getter: ()=>{return _this.fileName}
        })

        this.propertySheet.addProperty({
            key:"Start",
            type: PropertyType.BUTTON,
            action: this.startAudio.bind(this)
        })

        this.propertySheet.addProperty(
        {
            key:"Pause",
            type: PropertyType.BUTTON,
            action: this.stopAudio.bind(this)
        })
    }

    createElement(){
        if(!this.audioElement && this.loaded){
            this.audioElement = document.createElement("audio")
            this.audioElement.preload = "auto"
            let sourceEle = document.createElement("source") as HTMLSourceElement
            this.audioElement.appendChild(sourceEle)
            sourceEle.src = this.data
        }
    }

    startAudio(){
        this.createElement()
        this.audioElement.play()
    }

    stopAudio(){
        this.createElement()
        this.audioElement.pause()
    }

    getShapeName() {
        return shapeName
    }

    override update(){
        if(!this.loaded)
            return

        this.createElement()

        if(this.rawObj.IsVisible()){
            if(this.audioElement.paused){
                this.audioElement.play()
            }

            // Sync the time.
            let currentFrame = this.rawObj.GetLayer().GetCurrentFrame()
            let bornFrame = this.rawObj.GetBornFrameId()

            let elapsedTime = (currentFrame - bornFrame) / GlobalConfig.fps

            console.log("Global play time:" + elapsedTime + " music time:" + this.audioElement.currentTime)
            if(Math.abs(this.audioElement.currentTime - elapsedTime) > 0.1){
                console.log("Set music time to:" + elapsedTime)
                this.audioElement.currentTime = elapsedTime
            }
        } else {
            if(!this.audioElement.paused){
                this.audioElement.pause()
                this.audioElement.currentTime = 0.0
            }
        }
        super.update()
    }

    onDataLoaded() {
    }
}

shapeFactory.RegisterClass(shapeName, AudioShapeJS.createAudioShape)
export {AudioShapeJS}