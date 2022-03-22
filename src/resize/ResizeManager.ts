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

    private getRemainingSize(parentContainer: HTMLElement, isColumn: boolean): number {
        let parentContainerRect = parentContainer.getBoundingClientRect();
        let parentSize = isColumn ? parentContainerRect.height : parentContainerRect.width;
        let totalElementSize = DomHelper.getContainerChildSize(parentContainer, isColumn, ["hh-panel", "hh-container"])
        
        return parentSize - totalElementSize
    }

    public adjustSiblingsSize(splitter: HHSplitter, sizeDelta: number, isColumn: boolean) {
        let sibilingNodeNames = ['hh-container']

        let parentContainer = splitter.parentElement
        let parentContainerRect = Rect2D.fromDomRect(parentContainer.getBoundingClientRect())
        DomHelper.normalizeAllChildPanels(parentContainer, isColumn, sibilingNodeNames)

        let parentSize = isColumn ? parentContainerRect.height : parentContainerRect.width

        let prevSibiling = DomHelper.getPrevSiblingElementByName(splitter, sibilingNodeNames)
        DomHelper.increaseElementSize(prevSibiling, sizeDelta, parentSize, isColumn)

        let nextSibiling = DomHelper.getNextSiblingElementByName(splitter, sibilingNodeNames)

        if (nextSibiling) {
            DomHelper.increaseElementSize(nextSibiling, -sizeDelta, parentSize, isColumn)
        }

        let remainingSize = this.getRemainingSize(parentContainer, isColumn)
        if (nextSibiling) {
            DomHelper.increaseElementSize(nextSibiling, remainingSize, parentSize, isColumn)
        } else {
            DomHelper.increaseElementSize(prevSibiling, remainingSize, parentSize, isColumn)
        }
    }
}

export {ResizeManager}
