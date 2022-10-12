import {HHPanel} from "./HHPanel";
import {HHContainer} from "./HHContainer";
import {HHContent} from "./HHContent";
import {HHSideBar} from "./HHSideBar";

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

function findParentSideBar(ele): HHSideBar{
    return findParentElement(ele, HHSideBar)
}

export {findParentPanel, findParentContainer, findParentContent, findParentSideBar}