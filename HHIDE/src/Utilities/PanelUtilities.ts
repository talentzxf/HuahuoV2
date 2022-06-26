import {HHPanel} from "hhpanel"

function findParentPanel(ele) {
    let candidate = ele.parentElement
    while (candidate != null) {
        if (candidate instanceof HHPanel) {
            return candidate
        }
        candidate = candidate.parentElement
    }

    return null
}

export {findParentPanel}