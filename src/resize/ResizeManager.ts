import {HHPanel} from "../HHPanel";
import {Rect2D} from "../math/Rect2D";

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

    private normalizeAllChildPanels(parentContainer: HTMLElement, isColumn: boolean) {
        let parentContainerRect = parentContainer.getBoundingClientRect();
        let parentSize = isColumn?parentContainerRect.height:parentContainerRect.width;

        let panelSizeMap = new Map
        // Recalculate all the panel height percentage
        parentContainer.querySelectorAll('hh-panel').forEach(
            (panel: HHPanel) => {
                let panelRect2D: Rect2D = Rect2D.fromDomRect(panel.getBoundingClientRect())
                let panelSize = isColumn? panelRect2D.height: panelRect2D.width
                let sizePercentage = 100.0 * panelSize / parentSize
                panelSizeMap.set(panel, sizePercentage)
            }
        )
        panelSizeMap.forEach((sizePercentage: number, panel: HHPanel) => {
            if(isColumn)
                panel.style.height = sizePercentage + "%"
            else
                panel.style.width = sizePercentage + "%"
        })
    }

    private getRemainingSize(parentContainer: HTMLElement, isColumn: boolean): number {
        let parentContainerRect = parentContainer.getBoundingClientRect();
        let parentSize = isColumn?parentContainerRect.height:parentContainerRect.width;
        let totalPanelSize = 0
        parentContainer.querySelectorAll('hh-panel').forEach(
            (panel: HHPanel) => {
                let panelRect = panel.getBoundingClientRect()
                totalPanelSize += isColumn?panelRect.height:panelRect.width
            }
        )

        return parentSize - totalPanelSize
    }

    private increaseElementSize(ele: HTMLElement, amount: number, parentSize: number, isColumn: boolean){
        let boundingRect = ele.getBoundingClientRect()
        let oldSize: number = isColumn?boundingRect.height:boundingRect.width
        let newSize = oldSize + amount;
        let newSizePercentage = 100.0 * newSize / parentSize
        if(isColumn)
            ele.style.height = newSizePercentage + "%"
        else
            ele.style.width = newSizePercentage + "%"
    }

    public adjustPanelSiblingsSize(targetPanel: HHPanel, sizeDelta: number, isColumn: boolean) {
        let parentContainer = targetPanel.parentElement
        let parentContainerRect = Rect2D.fromDomRect(parentContainer.getBoundingClientRect())
        let parentSize = isColumn?parentContainerRect.height:parentContainerRect.width

        this.normalizeAllChildPanels(parentContainer, isColumn)

        this.increaseElementSize(targetPanel, sizeDelta, parentSize, isColumn)

        if (targetPanel.nextElementSibling) {
            this.increaseElementSize(targetPanel.nextElementSibling as HTMLElement, -sizeDelta, parentSize, isColumn)
        }

        let remainingSize = this.getRemainingSize(parentContainer, isColumn)
        if(targetPanel.nextElementSibling){
            this.increaseElementSize(targetPanel.nextElementSibling as HTMLElement, remainingSize, parentSize, isColumn)
        }else{
            this.increaseElementSize(targetPanel, remainingSize, parentSize, isColumn)
        }
    }
}

export {ResizeManager}
