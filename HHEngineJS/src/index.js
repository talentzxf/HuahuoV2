import {RenderEnginePaperJs} from "./RenderEngine/RenderEnginePaperImpl";
import {LineShapeJS} from "./Shapes/LineShapeJS"
import {RenderEngine2D} from "./RenderEngine/RenderEngine2D";
import {huahuoEngine} from "./EngineAPI";

let renderEngine2D = new RenderEnginePaperJs()

Module.onRuntimeInitialized = ()=>{
    huahuoEngine.OnInit()
}

export {renderEngine2D, LineShapeJS, huahuoEngine}