import {huahuoEngine, BaseShapeJS, shapeFactory} from "hhenginejs";
import {PlayerView} from "./PlayerView/PlayerView";
import {animationLoader} from "./PlayerView/AnimationLoader";

const queryString = window.location.search;
const urlParams = new URLSearchParams(queryString)
const projectId = urlParams.get("projectId")

if(projectId)
    animationLoader.loadAnimation(projectId)
else
    window.alert("Can't get projectId from URL")