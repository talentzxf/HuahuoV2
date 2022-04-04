import {HHContainer} from "hhpanel";
import {HHPanel} from 'hhpanel'
import {HHContent} from 'hhpanel'
import {HHTitle} from "hhpanel";

console.log(Module)
Module.onRuntimeInitialized = ()=>{
    var persistentManager = new Module.PersistentManager();
    console.log(persistentManager.getBufferSize())
}



