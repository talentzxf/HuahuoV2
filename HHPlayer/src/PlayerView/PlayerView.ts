import {CustomElement} from "hhcommoncomponents";
import {renderEngine2D, Player} from "hhenginejs";
import {huahuoEngine} from "hhenginejs"
import {eventBus} from "hhcommoncomponents";
import {AnimationLoaderEvents} from "./AnimationLoader";

@CustomElement({
    selector: "hh-player"
})
class PlayerView extends HTMLElement {
    inited: boolean = false
    canvasContainer: HTMLDivElement
    canvas: HTMLCanvasElement
    ctx: CanvasRenderingContext2D = null;

    animationPlayer: Player = null;
    storeId: number = null;

    readyCallbacks: Array<Function> = new Array<Function>()

    playImg:string = "data:image/svg+xml,%3C%3Fxml version='1.0' encoding='iso-8859-1'%3F%3E%3C!-- Generator: Adobe Illustrator 18.0.0  SVG Export Plug-In . SVG Version: 6.00 Build 0) --%3E%3C!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'%3E%3Csvg version='1.1' id='Capa_1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' x='0px' y='0px' viewBox='0 0 142.448 142.448' style='enable-background:new 0 0 142.448 142.448%3B' xml:space='preserve'%3E%3Cg%3E%3Cpath style='fill:%231D1D1B%3B' d='M142.411 68.9C141.216 31.48 110.968 1.233 73.549 0.038c-20.361-0.646-39.41 7.104-53.488 21.639C6.527 35.65-0.584 54.071 0.038 73.549c1.194 37.419 31.442 67.667 68.861 68.861c0.779 0.025 1.551 0.037 2.325 0.037c19.454 0 37.624-7.698 51.163-21.676C135.921 106.799 143.033 88.377 142.411 68.9z M111.613 110.336c-10.688 11.035-25.032 17.112-40.389 17.112c-0.614 0-1.228-0.01-1.847-0.029c-29.532-0.943-53.404-24.815-54.348-54.348c-0.491-15.382 5.122-29.928 15.806-40.958c10.688-11.035 25.032-17.112 40.389-17.112c0.614 0 1.228 0.01 1.847 0.029c29.532 0.943 53.404 24.815 54.348 54.348C127.91 84.76 122.296 99.306 111.613 110.336z'/%3E%3Cpath style='fill:%231D1D1B%3B' d='M94.585 67.086L63.001 44.44c-3.369-2.416-8.059-0.008-8.059 4.138v45.293c0 4.146 4.69 6.554 8.059 4.138l31.583-22.647C97.418 73.331 97.418 69.118 94.585 67.086z'/%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3C/svg%3E"
    pauseImg:string = "data:image/svg+xml,%3C%3Fxml version='1.0' encoding='iso-8859-1'%3F%3E%3C!-- Generator: Adobe Illustrator 19.0.0  SVG Export Plug-In . SVG Version: 6.00 Build 0) --%3E%3Csvg version='1.1' id='Capa_1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' x='0px' y='0px' viewBox='0 0 512 512' style='enable-background:new 0 0 512 512%3B' xml:space='preserve'%3E%3Cg%3E%3Cg%3E%3Cpath d='M256 0C114.842 0 0 114.842 0 256s114.842 256 256 256s256-114.842 256-256S397.158 0 256 0z M256 465.455c-115.493 0-209.455-93.961-209.455-209.455S140.507 46.545 256 46.545S465.455 140.507 465.455 256S371.493 465.455 256 465.455z'/%3E%3C/g%3E%3C/g%3E%3Cg%3E%3Cg%3E%3Cpath d='M318.061 139.636c-12.853 0-23.273 10.42-23.273 23.273v186.182c0 12.853 10.42 23.273 23.273 23.273c12.853 0 23.273-10.42 23.273-23.273V162.909C341.333 150.056 330.913 139.636 318.061 139.636z'/%3E%3C/g%3E%3C/g%3E%3Cg%3E%3Cg%3E%3Cpath d='M193.939 139.636c-12.853 0-23.273 10.42-23.273 23.273v186.182c0 12.853 10.42 23.273 23.273 23.273c12.853 0 23.273-10.42 23.273-23.273V162.909C217.212 150.056 206.792 139.636 193.939 139.636z'/%3E%3C/g%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3C/svg%3E"

    playButton: HTMLImageElement;
    constructor() {
        super();

        eventBus.addEventHandler("HHPlayer", AnimationLoaderEvents.LOADED, this.OnResize.bind(this))
    }
    
    executeAfterInit(callback:Function){
        if(this.inited){
            callback()
        }else{
            this.readyCallbacks.push(callback)
        }
    }

    createCanvasContainer() {
        this.canvasContainer = document.createElement("div")
        this.canvasContainer.id = "CanvasContainer"
        this.canvasContainer.style.width = "100%"
        this.canvasContainer.style.height = "100%"
        this.canvasContainer.style.padding = "0"
        this.canvasContainer.style.margin = "0"
        this.canvasContainer.style.backgroundColor = "gray"
        this.canvasContainer.style.flexBasis = "100%"
        this.canvasContainer.style.overflowY = "auto"

        this.appendChild(this.canvasContainer)
    }

    createCanvas() {
        this.canvas = document.createElement("canvas")
        this.canvas.setAttribute("resize", "true")
        this.canvas.style.padding = "0"
        this.canvas.style.margin = "0"
        this.canvasContainer.appendChild(this.canvas)

        this.ctx = this.canvas.getContext("2d")
    }

    OnResize() {
        if(window.getComputedStyle(this.parentElement).display == "none")
            return;

        let containerWidth = this.canvasContainer.clientWidth
        let containerHeight = this.canvasContainer.clientHeight
        let margin = 15
        let proposedCanvasWidth = containerWidth - margin
        let proposedCanvasHeight = containerHeight - margin
        // if(canvasHeight > containerHeight){
        //     canvasHeight = containerHeight * margin
        //     canvasWidth = canvasHeight * this.aspectRatio
        // }

        let [actualCanvasWidth, actualCanvasHeight] = renderEngine2D.getContentWH(proposedCanvasWidth, proposedCanvasHeight)

        renderEngine2D.resize(this.canvas, actualCanvasWidth, actualCanvasHeight)

        this.canvas.width = actualCanvasWidth
        this.canvas.height = actualCanvasHeight
        this.canvas.style.width = actualCanvasWidth + "px"
        this.canvas.style.height = actualCanvasHeight + "px"
        this.canvas.style.position = "relative"
        this.canvas.style.left = (containerWidth - actualCanvasWidth) / 2 + "px"
        this.canvas.style.top = (containerHeight - actualCanvasHeight) / 2 + "px"
        this.Redraw()
    }

    Redraw() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height)
        this.ctx.fillStyle = "white"
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)

        let previousCanvas = renderEngine2D.setDefaultCanvas(this.canvas)
        renderEngine2D.clearBackground()
        if(previousCanvas)
            renderEngine2D.setDefaultCanvas(previousCanvas)
    }

    createControllers(){
        this.playButton = document.createElement("img")
        this.playButton.src = this.playImg
        this.playButton.style.position = "absolute"
        this.playButton.style.width = "5%"
        this.playButton.style.height = "5%"
        this.playButton.style.left = "20px"
        this.playButton.style.top = "20px"

        this.playButton.addEventListener("click", this.onPlayButtonClicked.bind(this))

        this.appendChild(this.playButton)

        huahuoEngine.setActivePlayer(this.animationPlayer)
    }

    onPlayButtonClicked(){
        if(this.animationPlayer.isPlaying){
            this.animationPlayer.pausePlay()
            this.playButton.src = this.playImg
        }else{
            this.animationPlayer.startPlay()
            this.playButton.src = this.pauseImg
        }
    }

    connectedCallback(){
        if(!this.inited){
            this.createCanvasContainer()
            this.createCanvas()
            renderEngine2D.init(this.canvas, true)

            window.addEventListener("resize", this.OnResize.bind(this))

            let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
            resizeObserver.observe(this.canvasContainer)
            this.animationPlayer = new Player();
            this.animationPlayer.storeId = null

            this.style.width = "100%"
            this.style.height = "100%"

            this.createControllers()

            for(let callback of this.readyCallbacks){
                callback()
            }

            this.OnResize()

            this.inited = true
        }

    }
}

export {PlayerView}