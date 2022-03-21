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

    private normalizeAllChildPanels(parentContainer: HTMLElement, isColumn: boolean, nodeNames: Array<string>) {
        let parentContainerRect = parentContainer.getBoundingClientRect();
        let parentSize = isColumn ? parentContainerRect.height : parentContainerRect.width;

        let panelSizeMap = new Map
        let nodeNameString = nodeNames.map(element => {
            return element.toLowerCase()
        }).join(",")

        // Recalculate all the panel height percentage
        parentContainer.querySelectorAll(nodeNameString).forEach(
            (panel: HHPanel) => {
                let panelRect2D: Rect2D = Rect2D.fromDomRect(panel.getBoundingClientRect())
                let panelSize = isColumn ? panelRect2D.height : panelRect2D.width
                let sizePercentage = 100.0 * panelSize / parentSize
                panelSizeMap.set(panel, sizePercentage)
            }
        )
        panelSizeMap.forEach((sizePercentage: number, panel: HHPanel) => {
            if (isColumn)
                panel.style.height = sizePercentage + "%"
            else
                panel.style.width = sizePercentage + "%"
        })
    }

    private getRemainingSize(parentContainer: HTMLElement, isColumn: boolean): number {
        let parentContainerRect = parentContainer.getBoundingClientRect();
        let parentSize = isColumn ? parentContainerRect.height : parentContainerRect.width;
        let totalElementSize = 0
        parentContainer.childNodes.forEach(
            (ele: HTMLElement) => {
                if (ele.getBoundingClientRect) {
                    let eleRect = ele.getBoundingClientRect()
                    totalElementSize += isColumn ? eleRect.height : eleRect.width
                }
            }
        )

        return parentSize - totalElementSize
    }

    private increaseElementSize(ele: HTMLElement, amount: number, parentSize: number, isColumn: boolean) {
        let boundingRect = ele.getBoundingClientRect()
        let oldSize: number = isColumn ? boundingRect.height : boundingRect.width
        let newSize = oldSize + amount;
        let newSizePercentage = 100.0 * newSize / parentSize
        if (isColumn)
            ele.style.height = newSizePercentage + "%"
        else
            ele.style.width = newSizePercentage + "%"
    }

    public adjustSiblingsSize(splitter: HHSplitter, sizeDelta: number, isColumn: boolean) {
        let sibilingNodeNames = ['hh-container']

        let parentContainer = splitter.parentElement
        let parentContainerRect = Rect2D.fromDomRect(parentContainer.getBoundingClientRect())
        this.normalizeAllChildPanels(parentContainer, isColumn, sibilingNodeNames)

        let parentSize = isColumn ? parentContainerRect.height : parentContainerRect.width

        let prevSibiling = DomHelper.getPrevSiblingElementByName(splitter, sibilingNodeNames)
        this.increaseElementSize(prevSibiling, sizeDelta, parentSize, isColumn)

        let nextSibiling = DomHelper.getNextSiblingElementByName(splitter, sibilingNodeNames)

        if (nextSibiling) {
            this.increaseElementSize(nextSibiling, -sizeDelta, parentSize, isColumn)
        }

        let remainingSize = this.getRemainingSize(parentContainer, isColumn)
        if (nextSibiling) {
            this.increaseElementSize(nextSibiling, remainingSize, parentSize, isColumn)
        } else {
            this.increaseElementSize(prevSibiling, remainingSize, parentSize, isColumn)
        }
    }
}

export {ResizeManager}
