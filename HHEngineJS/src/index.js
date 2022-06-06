import {RenderEnginePaperJs} from "./RenderEngine/RenderEnginePaperImpl";
import {LineShapeJS} from "./Shapes/LineShapeJS"
import {huahuoEngine} from "./EngineAPI";

let renderEngine2D = new RenderEnginePaperJs()

Module.onRuntimeInitialized = ()=>{
    huahuoEngine.OnInit()
}

export {renderEngine2D, LineShapeJS, huahuoEngine}