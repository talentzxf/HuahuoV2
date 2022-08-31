import {huahuoEngine} from "hhenginejs";

function init(){
    huahuoEngine.ExecuteAfterInited(()=>{
        let loadingElement = document.querySelector("#loading") as HTMLDivElement
        loadingElement.style.display = "none"
        let mainDiv = document.querySelector("#mainScene") as HTMLDivElement
        mainDiv.style.display = "block"
    })

}

export {init}