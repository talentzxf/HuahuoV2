import {huahuoEngine} from "hhenginejs";

function showMainDiv(){
    let loadingElement = document.querySelector("#loading") as HTMLDivElement
    loadingElement.style.display = "none"
    let mainDiv = document.querySelector("#mainSceneDiv") as HTMLDivElement
    mainDiv.style.display = "block"
}

let i18n = (window as any).i18n

function init(){
    huahuoEngine.ExecuteAfterInited(()=>{
        i18n.ExecuteAfterInited(()=>{
            showMainDiv()
        })
    })
}

export {init}