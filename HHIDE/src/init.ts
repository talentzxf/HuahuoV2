import {huahuoEngine, renderEngine2D} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";
import {shortcutsManager} from "./Shortcuts/ShortcutsManager";

function showMainDiv(){
    let loadingElement = document.querySelector("#loading") as HTMLDivElement
    loadingElement.style.display = "none"
    let mainDiv = document.querySelector("#mainSceneDiv") as HTMLDivElement
    mainDiv.style.display = "block"
}

let i18n = (window as any).i18n

function updateMousePos(evt:MouseEvent){
    let posX = evt.offsetX
    let posY = evt.offsetY

    let globalPosition = renderEngine2D.getGlobalPosition(posX, posY)

    let statusBarSpan = document.querySelector("#statusBarSpan")
    statusBarSpan.innerHTML = "MousePosition:" + Math.round(globalPosition.x) + "," + Math.round(globalPosition.y)
}

function init(){
    huahuoEngine.ExecuteAfterInited(()=>{
        i18n.ExecuteAfterInited(()=>{
            HHToast.info(i18n.t("welcomeMsg"))
            showMainDiv()

            document.body.addEventListener("mousemove", updateMousePos)

            shortcutsManager.init()
        })
    })
}

export {init}