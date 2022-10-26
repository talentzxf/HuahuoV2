import {HHToast} from "hhcommoncomponents";
import {PlayerView} from "./PlayerView/PlayerView";

import {huahuoEngine} from "hhenginejs";
import {animationLoader} from "./PlayerView/AnimationLoader";
import {init} from "./init";

const queryString = window.location.search;
const urlParams = new URLSearchParams(queryString)
const projectId = urlParams.get("projectId")

init()

huahuoEngine.ExecuteAfterInited(()=>{
    if(projectId)
        animationLoader.loadAnimation(projectId)
    else
        window.alert("Can't get projectId from URL")
})