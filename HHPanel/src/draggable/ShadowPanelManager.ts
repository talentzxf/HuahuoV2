import {Rect2D} from "../math/Rect2D";

class ShadowPanelManager{
    private shadowPanelDiv:HTMLDivElement
    static Bar: number = 0.3;
    private constructor() {
        this.shadowPanelDiv = document.createElement("div") as HTMLDivElement
        document.body.appendChild(this.shadowPanelDiv)
        this.shadowPanelDiv.style.backgroundColor = "gray"
        this.shadowPanelDiv.style.position = "absolute"
        this.shadowPanelDiv.style.opacity = "30%"
    }

    private static instance:ShadowPanelManager

    public static getInstance(): ShadowPanelManager{
        if(ShadowPanelManager.instance == null){
            ShadowPanelManager.instance = new ShadowPanelManager()
        }
        return ShadowPanelManager.instance
    }

    updateShadowPanel(shadowPanelRect: Rect2D) {
        this.shadowPanelDiv.style.visibility = 'unset'
        this.shadowPanelDiv.style.left = shadowPanelRect.getLeftUp().X + "px"
        this.shadowPanelDiv.style.top = shadowPanelRect.getLeftUp().Y + "px"
        this.shadowPanelDiv.style.width = shadowPanelRect.width + "px"
        this.shadowPanelDiv.style.height = shadowPanelRect.height + "px"
    }

    hideShadowPanel(){
        this.shadowPanelDiv.style.visibility = 'hidden'
    }
}

export {ShadowPanelManager}