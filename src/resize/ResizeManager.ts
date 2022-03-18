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

    private normalizeAllChildPanels(parentContainer: HTMLElement) {
        let parentHeight = parentContainer.getBoundingClientRect().height
        let panelHeightMap = new Map
        // Recalculate all the panel height percentage
        parentContainer.querySelectorAll('hh-panel').forEach(
            (panel: HHPanel) => {
                let panelRect2D: Rect2D = Rect2D.fromDomRect(panel.getBoundingClientRect())
                let heightPercentage = 100.0 * panelRect2D.height / parentHeight
                panelHeightMap.set(panel, heightPercentage)
            }
        )
        panelHeightMap.forEach((heightPercentage: number, panel: HHPanel) => {
            panel.style.height = heightPercentage + "%"
        })
    }

    private getRemainingHeight(parentContainer: HTMLElement): number {
        let parentHeight = parentContainer.getBoundingClientRect().height
        let totalPanelHeight = 0
        parentContainer.querySelectorAll('hh-panel').forEach(
            (panel: HHPanel) => {
                totalPanelHeight += panel.getBoundingClientRect().height
            }
        )

        return parentHeight - totalPanelHeight
    }

    private increaseElementHeight(ele: HTMLElement, amount: number, parentHeight: number){
        let oldHeight: number = ele.getBoundingClientRect().height
        let newHeight = oldHeight + amount;
        let newHeightPercentage = 100.0 * newHeight / parentHeight
        ele.style.height = newHeightPercentage + "%"
    }

    public adjustPanelSiblingsHeight(targetPanel: HHPanel, heightDelta: number) {
        let parentContainer = targetPanel.parentElement
        let parentContainerRect = Rect2D.fromDomRect(parentContainer.getBoundingClientRect())
        let parentHeight = parentContainerRect.height

        this.normalizeAllChildPanels(parentContainer)

        this.increaseElementHeight(targetPanel, heightDelta, parentHeight)

        if (targetPanel.nextElementSibling) {
            this.increaseElementHeight(targetPanel.nextElementSibling as HTMLElement, -heightDelta, parentHeight)
        }

        let remainingHeight = this.getRemainingHeight(parentContainer)
        if(targetPanel.nextElementSibling){
            this.increaseElementHeight(targetPanel.nextElementSibling as HTMLElement, remainingHeight, parentHeight)
        }else{
            this.increaseElementHeight(targetPanel, remainingHeight, parentHeight)
        }

        // let targetPanelRect = Rect2D.fromDomRect(targetPanel.getBoundingClientRect())
        //
        // let nextPanel = targetPanel.nextElementSibling as HHPanel

    }
}

export {ResizeManager}
