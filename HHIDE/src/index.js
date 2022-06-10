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
    let Uint8Array = Module.writeObjectStoreInMemoryFile()
    let blob = new Blob([Uint8Array], {type:"application/octet-stream"})
    saveAs(blob, "huahuo.data")
    Logger.info("Good!!")
}

function load(e){
    var file = e.target.files[0];
    var reader = new FileReader()
    reader.onload = function(e){
        let fileContent = new Uint8Array(e.target.result);
        let storeMemoryFile = "mem://objectstore.data"
        let fileSize = fileContent.length;
        let memoryFileContent = Module.createMemFile(storeMemoryFile, fileSize);
        for(let i = 0 ; i < fileSize; i++){ // Copy to the file, byte by byte
            memoryFileContent[i] = fileContent[i];
        }

        Module.LoadFileCompletely(storeMemoryFile);

    }
    reader.readAsArrayBuffer(file)
}

window.menuoperations = {
    save: save,
    load: load
}
