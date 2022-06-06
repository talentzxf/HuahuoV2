import {HHContainer} from "hhpanel";
import {HHPanel} from 'hhpanel'
import {HHContent} from 'hhpanel'
import {HHTitle} from "hhpanel";
import {HHTimeline} from "hhtimeline";
import {NavTree} from "./UIComponents/NavTree"
import {DrawToolBar} from "./UIComponents/DrawToolBar";
import {SceneView} from "./SceneView/SceneView";

import { library, dom } from "@fortawesome/fontawesome-svg-core";
import { faPlus } from "@fortawesome/free-solid-svg-icons/faPlus";
import {faMinus} from "@fortawesome/free-solid-svg-icons/faMinus"
import {faSlash} from "@fortawesome/free-solid-svg-icons/faSlash";
import {faCircle} from "@fortawesome/free-solid-svg-icons/faCircle";
import {faBezierCurve} from "@fortawesome/free-solid-svg-icons/faBezierCurve";
import { saveAs } from 'file-saver';


library.add(faMinus)
library.add(faPlus)
library.add(faSlash)
library.add(faCircle)
library.add(faBezierCurve)
dom.watch();

function save(){
    let int8Array = Module.WritePersistentManagerInMemory()
    let blob = new Blob([int8Array], {type:"application/octet-stream"})
    saveAs(blob, "huahuo.data")
    Logger.info("Good!!")
}

window.menuoperations = {
    save: save
}
