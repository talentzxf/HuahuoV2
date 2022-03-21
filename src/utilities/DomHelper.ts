import {HHPanel} from "../HHPanel";
import {Rect2D} from "../math/Rect2D";

class DomHelper {
    private constructor() {
    }

    public static increaseElementSize(ele: HTMLElement, amount: number, parentSize: number, isColumn: boolean) {
        let boundingRect = ele.getBoundingClientRect()
        let oldSize: number = isColumn ? boundingRect.height : boundingRect.width
        let newSize = oldSize + amount;
        let newSizePercentage = 100.0 * newSize / parentSize
        if (isColumn)
            ele.style.height = newSizePercentage + "%"
        else
            ele.style.width = newSizePercentage + "%"
    }


    public static normalizeAllChildPanels(parentContainer: HTMLElement, isColumn: boolean, nodeNames: Array<string>) {
        let parentContainerRect = parentContainer.getBoundingClientRect();
        let parentSize = isColumn ? parentContainerRect.height : parentContainerRect.width;

        let panelSizeMap = new Map
        // Recalculate all the panel height percentage
        DomHelper.getChildElements(parentContainer, nodeNames).forEach(
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

    static getNextSiblingElementByName(curElement: HTMLElement, elementNames: Array<string>): HTMLElement {
        let nextSibling: HTMLElement = curElement.nextElementSibling as HTMLElement

        let foundElement: HTMLElement
        while (nextSibling) {
            let nextSiblingNodeName = nextSibling.nodeName.toLowerCase()
            elementNames.forEach(elementName => {
                if (nextSiblingNodeName == elementName.toLowerCase())
                    foundElement = nextSibling
            })

            if (foundElement)
                return foundElement
            nextSibling = nextSibling.nextElementSibling as HTMLElement
        }

        return nextSibling
    }

    static getPrevSiblingElementByName(curElement: HTMLElement, elementNames: Array<string>) {
        let prevSibiling: HTMLElement = curElement.previousElementSibling as HTMLElement
        let foundElement: HTMLElement
        while (prevSibiling) {
            let nextSiblingNodeName = prevSibiling.nodeName.toLowerCase()
            elementNames.forEach(elementName => {
                if (nextSiblingNodeName == elementName.toLowerCase())
                    foundElement = prevSibiling
            })

            if (foundElement)
                return foundElement
            prevSibiling = prevSibiling.previousElementSibling as HTMLElement
        }

        return prevSibiling
    }

    static getChildElements(ele: HTMLElement, nodeNames: Array<string>):Array<HTMLElement> {
        let returnElements = new Array()

        let childElement = ele.firstElementChild
        while(childElement != null){
            let nodeNameLC = childElement.nodeName.toLowerCase()
            for(let nodeName in nodeNames){
                if(nodeNameLC == nodeName.toLowerCase()){
                    returnElements.push(childElement)
                }
            }
            childElement = childElement.nextElementSibling
        }

        return returnElements
    }
}

export {DomHelper}