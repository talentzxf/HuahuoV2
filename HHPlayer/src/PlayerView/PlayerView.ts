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
        // this.canvasContainer.style.width = "100%"
        // this.canvasContainer.style.height = "100%"
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

    connectedCallback(){
        if(!this.inited){
            this.style.width = "100%"
            this.style.height = "100%"

            this.createCanvasContainer()
            this.createCanvas()
            renderEngine2D.init(this.canvas)

            this.animationPlayer = new Player();

            for(let callback of this.readyCallbacks){
                callback()
            }

            this.inited = true
        }

    }
}

export {PlayerView}