import {NailManager} from "./NailManager";

function getNailManager(): NailManager {
    let nailManager = window["NailManagerJS"]
    if (!nailManager) {
        nailManager = new NailManager()
        window["NailManagerJS"] = nailManager
    }

    return nailManager
}

export {getNailManager}