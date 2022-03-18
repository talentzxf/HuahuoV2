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

    private normalizeAllChildPanels(parentContainer:HTMLElement){
        let parentContainerRect = Rect2D.fromDomRect(parentContainer.getBoundingClientRect())
        let parentHeight = parentContainerRect.height

        let panelHeightMap = new Map
        // Recalculate all the panel height percentage
        parentContainer.querySelectorAll('hh-panel').forEach(
            (panel:HHPanel) => {
                let panelRect2D:Rect2D = Rect2D.fromDomRect(panel.getBoundingClientRect())
                let heightPercentage = 100.0 * panelRect2D.height / parentHeight
                panelHeightMap.set(panel, heightPercentage)
            }
        )
        panelHeightMap.forEach((heightPercentage:number, panel:HHPanel) => {
            panel.style.height = heightPercentage + "%"
        })
    }

    public adjustPanelSiblingsHeight(targetPanel: HHPanel, newHeight: number) {
        let parentContainer = targetPanel.parentElement
        let parentContainerRect = Rect2D.fromDomRect(parentContainer.getBoundingClientRect())
        let parentHeight = parentContainerRect.height

        this.normalizeAllChildPanels(parentContainer)

        let oldHeight: number = targetPanel.getBoundingClientRect().height
        let newHeightPercentage = 100.0 * newHeight / parentHeight
        targetPanel.style.height = newHeightPercentage + "%"

        if(targetPanel.nextElementSibling){
            let growHeight = newHeight - oldHeight
            let siblingElement = targetPanel.nextElementSibling as HHPanel
            let oldSiblingHeight = siblingElement.getBoundingClientRect().height
            let newSiblingHeight = oldSiblingHeight - growHeight
            siblingElement.style.height = 100.0 * newSiblingHeight/parentHeight + "%"
        }

        // let targetPanelRect = Rect2D.fromDomRect(targetPanel.getBoundingClientRect())
        //
        // let nextPanel = targetPanel.nextElementSibling as HHPanel

    }
}

export {ResizeManager}
