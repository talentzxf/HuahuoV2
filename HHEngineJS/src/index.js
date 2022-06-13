import {RenderEnginePaperJs} from "./RenderEngine/RenderEnginePaperImpl";
import {LineShapeJS} from "./Shapes/LineShapeJS"
import {CircleShapeJS} from "./Shapes/CircleShapeJS"
import {huahuoEngine} from "./EngineAPI";

let renderEngine2D = new RenderEnginePaperJs()

Module.onRuntimeInitialized = ()=>{
    huahuoEngine.OnInit()
}

export {renderEngine2D, LineShapeJS, CircleShapeJS, huahuoEngine}