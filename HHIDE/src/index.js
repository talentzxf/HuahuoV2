import {HHContainer} from "hhpanel";
import {HHPanel} from 'hhpanel'
import {HHContent} from 'hhpanel'
import {HHTitle} from "hhpanel";

console.log(Module)
Module.onRuntimeInitialized = ()=>{
    var persistentManager = Module.PersistentManager.prototype.getInstance();
    console.log(persistentManager.getBufferSize())
    console.log(persistentManager.getBuffer().getByte(0))
    console.log(persistentManager.getBuffer().getByte(1))
    console.log(persistentManager.getBuffer().getByte(2))
    console.log(persistentManager.getBuffer().getByte(3))
}



