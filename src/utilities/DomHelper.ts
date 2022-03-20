class DomHelper {
    private constructor() {
    }

    static getNextSiblingElementByName(curElement: HTMLElement, elementName: String): HTMLElement {
        let nextSibling: HTMLElement = curElement.nextElementSibling as HTMLElement
        while (nextSibling) {
            if (nextSibling.nodeName.toLowerCase() == elementName.toLowerCase())
                return nextSibling
            nextSibling = nextSibling.nextElementSibling as HTMLElement
        }

        return nextSibling
    }

    static getPrevSiblingElementByName(curElement: HTMLElement, elementName: string) {
        let prevSibiling: HTMLElement = curElement.previousElementSibling as HTMLElement
        while (prevSibiling) {
            if (prevSibiling.nodeName.toLowerCase() == elementName.toLowerCase())
                return prevSibiling
            prevSibiling = prevSibiling.nextElementSibling as HTMLElement
        }

        return prevSibiling
    }
}

export {DomHelper}