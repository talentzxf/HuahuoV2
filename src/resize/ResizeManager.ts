import {HHPanel} from "../HHPanel";
import {Rect2D} from "../math/Rect2D";
import {HHSplitter} from "../HHSplitter";
import {DomHelper} from "../utilities/DomHelper";

class ResizeManager {
    private static Instance: ResizeManager;

    private constructor() {
    }

    public static getInstance(): ResizeManager {
        if (ResizeManager.Instance == null) {
            ResizeManager.Instance = new ResizeManager()
        }
        return ResizeManager.Instance
    }

    public adjustSiblingsSize(splitter: HHSplitter, sizeDelta: number, isColumn: boolean) {
        let siblingNodeNames = ['hh-container']

        let parentContainer = splitter.parentElement
        let parentContainerRect = Rect2D.fromDomRect(parentContainer.getBoundingClientRect())
        DomHelper.normalizeAllChildPanels(parentContainer, isColumn, siblingNodeNames)

        let parentSize = isColumn ? parentContainerRect.height : parentContainerRect.width

        let prevSibling = DomHelper.getPrevSiblingElementByName(splitter, siblingNodeNames)
        DomHelper.increaseElementSize(prevSibling, sizeDelta, parentSize, isColumn)

        let nextSibling = DomHelper.getNextSiblingElementByName(splitter, siblingNodeNames)

        if (nextSibling) {
            DomHelper.increaseElementSize(nextSibling, -sizeDelta, parentSize, isColumn)
        }

        let remainingSize = DomHelper.getRemainingSize(parentContainer, isColumn)
        if (nextSibling) {
            DomHelper.increaseElementSize(nextSibling, remainingSize, parentSize, isColumn)
        } else {
            DomHelper.increaseElementSize(prevSibling, remainingSize, parentSize, isColumn)
        }
    }
}

export {ResizeManager}
