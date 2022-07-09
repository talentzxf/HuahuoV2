import {HHPanel, HHContainer, HHContent} from "hhpanel"

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

function findParentPanel(ele) : HHPanel{
    return findParentElement(ele, HHPanel)
}

function findParentContainer(ele): HHContainer{
    return findParentElement(ele, HHContainer)
}

function findParentContent(ele): HHContent{
    return findParentElement(ele, HHContent)
}

export {findParentPanel, findParentContainer, findParentContent}