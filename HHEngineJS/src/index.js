import {RenderEnginePaperJs} from "./RenderEngine/RenderEnginePaperImpl";
import {LineShapeJS} from "./Shapes/LineShapeJS"
import {CircleShapeJS} from "./Shapes/CircleShapeJS"
import {RectangleJS} from "./Shapes/RectangleJS";
import {huahuoEngine} from "./EngineAPI";
import * as paper from "paper"

let renderEngine2D = new RenderEnginePaperJs()

if(Module.IsWASMInited && Module.IsWASMInited()){
    console.log("Init right now")
    huahuoEngine.OnInit()
} else {
    console.log("Init later")
    Module.onRuntimeInitialized = ()=>{
        console.log("Init now!")
        huahuoEngine.OnInit()
    }
}

window.enginejsInited = true
export {renderEngine2D, LineShapeJS, CircleShapeJS,RectangleJS, huahuoEngine, paper}