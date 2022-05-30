import {HHContainer} from "hhpanel";
import {HHPanel} from 'hhpanel'
import {HHContent} from 'hhpanel'
import {HHTitle} from "hhpanel";
import {HHTimeline} from "hhtimeline";
import {NavTree} from "./UIComponents/NavTree"


import { library, dom } from "@fortawesome/fontawesome-svg-core";
import { faPlus } from "@fortawesome/free-solid-svg-icons/faPlus";
import {faMinus} from "@fortawesome/free-solid-svg-icons/faMinus"
import {EngineAPI} from "./EngineAPI";

library.add(faMinus)
library.add(faPlus)
dom.watch();

Module.onRuntimeInitialized = ()=>{
    Module.HuaHuoEngine.prototype.InitEngine();
    EngineAPI.OnInit()
    Module["SceneView"].prototype.GetSceneView().InitWithCanvasId("SceneView")
}
