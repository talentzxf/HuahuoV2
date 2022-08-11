import {CustomElement} from "hhcommoncomponents";
import {renderEngine2D, Player} from "hhenginejs";

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
        let canvasWidth = containerWidth - margin
        let canvasHeight = containerHeight - margin

        renderEngine2D.resize(this.canvas, canvasWidth, canvasHeight)

        // if(canvasHeight > containerHeight){
        //     canvasHeight = containerHeight * margin
        //     canvasWidth = canvasHeight * this.aspectRatio
        // }

        this.canvas.width = canvasWidth
        this.canvas.height = canvasHeight
        this.canvas.style.width = canvasWidth + "px"
        this.canvas.style.height = canvasHeight + "px"
        this.canvas.style.position = "relative"
        this.canvas.style.left = (containerWidth - canvasWidth) / 2 + "px"
        this.canvas.style.top = (containerHeight - canvasHeight) / 2 + "px"
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

    connectedCallback(){
        if(!this.inited){
            this.style.width = "100%"
            this.style.height = "100%"

            this.createCanvasContainer()
            this.createCanvas()
            renderEngine2D.init(this.canvas)

            window.addEventListener("resize", this.OnResize.bind(this))

            let resizeObserver = new ResizeObserver(this.OnResize.bind(this))
            resizeObserver.observe(this.canvasContainer)


            this.animationPlayer = new Player();

            for(let callback of this.readyCallbacks){
                callback()
            }

            this.inited = true
        }

    }
}

export {PlayerView}