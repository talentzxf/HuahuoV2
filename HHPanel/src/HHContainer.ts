import {CustomElement} from "hhcommoncomponents";
import {DomHelper} from "./utilities/DomHelper";
import {findParentContainer} from "./PanelUtilities";

@CustomElement({
    selector: "hh-container",
})
class HHContainer extends HTMLElement {

    private get direction(): string {
        return this.getAttribute("direction")
    }

    private get size(): string {
        return this.getAttribute("size")
    }

    private get hasSplitter(): boolean {
        let hasSplitter = this.getAttribute("hasSplitter")
        if (!hasSplitter || hasSplitter == "true")
            return true
        if (hasSplitter == "false")
            return false
        throw "hasSplitter can either be true or false"
    }

    private get isColumn(): boolean {
        let dir: string = this.direction;
        if (dir == null || dir == "column") return true
        if (dir == "row") return false
        throw "Unknown direction, should be: column or row."
    }

    constructor() {
        super();
    }

    updateDirection() {

    }

    connectedCallback() {
        this.style.display = "flex"
        this.style.overflowY = "auto"

        if (this.size != "fit-content") {
            if (this.style.width == "") {
                this.style.width = "100%"
            }

            if (this.style.height == "") {
                this.style.height = "100%"
            }
        }

        this.style.flexDirection = this.isColumn ? "column" : "row"

        if (this.hasSplitter) {
            let nextElement = this.nextElementSibling
            if (nextElement != null && nextElement.nodeName.toLowerCase() == "hh-splitter")
                return

            let nextSibling = DomHelper.getNextSiblingElementByName(this, ["hh-container"])
            if (nextSibling) {
                let splitter = document.createElement("hh-splitter")
                splitter.setAttribute("direction", this.parentElement.style.flexDirection)
                this.parentElement.insertBefore(splitter, nextSibling)
            }
        }
    }

    attributeChangedCallback(name: String, oldValue: any, newValue: any) {
        if (name.toLowerCase() == "direction") {
            this.style.flexDirection = newValue
        }
    }

    hide() {
        this.style.display = "none"

        let parentContainer = findParentContainer(this)
        parentContainer.distributeChildrenEvenly()
    }

    show() {
        this.style.display = "flex"

        let parentContainer = findParentContainer(this)
        parentContainer.distributeChildrenEvenly()
    }

    distributeChildrenEvenly() {
        let totalSize = this.isColumn ? this.clientHeight : this.clientWidth;

        let children = DomHelper.getChildElements(this, ["hh-container"])

        let splitters = DomHelper.getChildElements(this, ["hh-splitter"])

        let totalVisibleChildrenCount = 0
        for (let child of children) {
            if (child.style.display != "none")
                totalVisibleChildrenCount++
        }

        if (totalVisibleChildrenCount == 0) { //Avoid 0/0
            return
        }

        let _this = this
        let totalSplitterSize = 0
        splitters.forEach((splitter) => {
            let splitterSize = _this.isColumn ? splitter.offsetHeight : splitter.offsetWidth
            totalSplitterSize += splitterSize
        })

        let totalUsableSize = totalSize - totalSplitterSize
        let eachChildSize = totalUsableSize / totalVisibleChildrenCount

        let eachChildPercentage = (100.0 * eachChildSize / totalSize) + "%"

        for (let child of children) {
            if (child.style.display != "none") {
                if (this.isColumn) {
                    child.style.height = eachChildPercentage
                } else {
                    child.style.width = eachChildPercentage
                }
            }
        }
    }
}

export {HHContainer}