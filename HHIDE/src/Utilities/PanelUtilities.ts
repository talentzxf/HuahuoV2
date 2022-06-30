import {HHPanel, HHContainer} from "hhpanel"

function findParentElement(ele, clz){
    let candidate = ele.parentElement
    while (candidate != null) {
        if (candidate instanceof clz) {
            return candidate
        }
        candidate = candidate.parentElement
    }

    return null
}

function findParentPanel(ele) {
    return findParentElement(ele, HHPanel)
}

function findParentContainer(ele){
    return findParentElement(ele, HHContainer)
}

export {findParentPanel, findParentContainer}