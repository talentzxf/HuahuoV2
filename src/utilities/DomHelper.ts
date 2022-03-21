class DomHelper {
    private constructor() {
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
            prevSibiling = prevSibiling.nextElementSibling as HTMLElement
        }

        return prevSibiling
    }
}

export {DomHelper}