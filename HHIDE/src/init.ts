import {huahuoEngine} from "hhenginejs";
import i18next from "i18next";

function showMainDiv(){
    let loadingElement = document.querySelector("#loading") as HTMLDivElement
    loadingElement.style.display = "none"
    let mainDiv = document.querySelector("#mainScene") as HTMLDivElement
    mainDiv.style.display = "block"
}
function init(){
    huahuoEngine.ExecuteAfterInited(()=>{
        if(i18next.isInitialized)
            showMainDiv()
        else{
            i18next.on("loaded", function(load){
                if(load.hasOwnProperty("en")){
                    showMainDiv()
                }
            })
        }

    })
}

export {init}