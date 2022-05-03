import {HHContainer} from "hhpanel";
import {HHPanel} from 'hhpanel'
import {HHContent} from 'hhpanel'
import {HHTitle} from "hhpanel";
import {NavTree} from "./UIComponents/NavTree"

import { library, dom } from "@fortawesome/fontawesome-svg-core";
import { faPlus } from "@fortawesome/free-solid-svg-icons/faPlus";
import {faMinus} from "@fortawesome/free-solid-svg-icons/faMinus"

library.add(faMinus)
library.add(faPlus)
dom.watch();

console.log(Module)
Module.onRuntimeInitialized = ()=>{
    Module.HuaHuoEngine.prototype.InitEngine();
    var huaHuoEngine = Module.HuaHuoEngine.prototype.getInstance();
    var gameObject = huaHuoEngine.CreateGameObject("Hello");
    console.log(gameObject.GetName())
}
