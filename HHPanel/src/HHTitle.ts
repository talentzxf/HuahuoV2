import {Vector2D} from "./math/Vector2D";
import {MovableElement, TabMover} from "./draggable/TabMover";
import {CustomElement} from "hhcommoncomponents";
import {OccupiedTitleManager} from "./draggable/OccupiedTitleManager";
import {HHPanel} from "./HHPanel";
import {HHContent} from "./HHContent";

let closeButtonSvg: string = "data:image/svg+xml,%3C%3Fxml version='1.0' encoding='utf-8'%3F%3E%3C!-- Generator: Adobe Illustrator 17.1.0  SVG Export Plug-In . SVG Version: 6.00 Build 0) --%3E%3C!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'%3E%3Csvg version='1.1' id='Layer_1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' x='0px' y='0px' viewBox='0 0 50 50' enable-background='new 0 0 50 50' xml:space='preserve'%3E%3Cpath fill='%23231F20' d='M9.016 40.837c0.195 0.195 0.451 0.292 0.707 0.292c0.256 0 0.512-0.098 0.708-0.293l14.292-14.309l14.292 14.309c0.195 0.196 0.451 0.293 0.708 0.293c0.256 0 0.512-0.098 0.707-0.292c0.391-0.39 0.391-1.023 0.001-1.414L26.153 25.129L40.43 10.836c0.39-0.391 0.39-1.024-0.001-1.414c-0.392-0.391-1.024-0.391-1.414 0.001L24.722 23.732L10.43 9.423c-0.391-0.391-1.024-0.391-1.414-0.001c-0.391 0.39-0.391 1.023-0.001 1.414l14.276 14.293L9.015 39.423C8.625 39.813 8.625 40.447 9.016 40.837z'/%3E%3C/svg%3E"

@CustomElement({
    selector: "hh-title",
})
class HHTitle extends HTMLElement implements MovableElement {
    private startMoving: Boolean = false
    private isMoving: Boolean = false
    private startElePos: Vector2D = new Vector2D()
    private startPos: Vector2D
    private parentPanel: HHPanel
    private content: HHContent

    currentlyDockedElement: HTMLElement; // No use for HHTitle.

    private inited: boolean = false;

    private isVertical: boolean = false

    static get tabIndex() {
        return ['tabindex']
    }

    constructor() {
        super();

        this.addEventListener("mousedown", this.mouseDown)
    }

    setIsVertical(isVertical: boolean) {
        this.isVertical = isVertical
    }

    getContent(): HHContent {
        return this.content
    }

    setContent(inContent: HHContent) {
        this.content = inContent
        this.content.setTitle(this)
    }

    setParentPanel(panel: HHPanel) {
        this.parentPanel = panel
        panel.getTabGroup().appendChild(this)
        panel.getContentGroup().appendChild(this.content)
    }

    mouseDown(evt: MouseEvent) {
        this.startPos = new Vector2D(evt.clientX, evt.clientY)
        this.startMoving = true
        this.isMoving = false
        // this.startElePos = new Vector2D(this.offsetLeft, this.offsetTop);
        let boundingRect = this.getBoundingClientRect()
        this.startElePos = new Vector2D(boundingRect.left, boundingRect.top)
        document.onmousemove = this.mouseMove.bind(this)
        document.onmouseup = this.mouseMove.bind(this)
    }

    connectedCallback() {
        if (!this.inited) {
            this.style.display = "flex"

            this.style.width = "fit-content"
            this.style.height = "fit-content"

            let closableStr = this.getContent().getAttribute("closable")

            let closable = true
            if (closableStr == "false") {
                closable = false
            }

            if (closable) {
                // console.log("Title connectedCallback")
                let closeButton = document.createElement("img")
                closeButton.src = closeButtonSvg
                closeButton.onclick = this.close.bind(this)

                let closeButtonWidth = Math.min(this.offsetWidth, this.offsetHeight) * 0.5
                closeButton.style.width = closeButtonWidth + "px"
                closeButton.style.height = closeButtonWidth + "px"

                this.appendChild(closeButton)
                this.inited = true
            }
        }
    }

    close(e: MouseEvent) {
        e.stopPropagation()
        e.preventDefault()

        // Close this tab
        this.parentPanel.closeTab(this.tabIndex)

        // Open the previous tab
        let candidateTabIndex = this.tabIndex - 1
        while (candidateTabIndex >= 0) {
            if (this.parentPanel.isValidTabIndex(candidateTabIndex)) {
                this.parentPanel.selectTab(candidateTabIndex)
                return
            }
            candidateTabIndex--;
        }

        // Can't find previous tab, open next tab

        candidateTabIndex = this.tabIndex + 1
        while (candidateTabIndex <= this.parentPanel.maxTabId) {
            if (this.parentPanel.isValidTabIndex(candidateTabIndex)) {
                this.parentPanel.selectTab(candidateTabIndex)
                return
            }
            candidateTabIndex++;
        }
    }

    mouseMove(evt: MouseEvent) {
        if (evt.buttons == 1) {
            if (this.startMoving && !this.startPos.equals(evt.clientX, evt.clientY)) {
                this.isMoving = true
            }

            if (this.isMoving) {
                let offsetX = evt.clientX - this.startPos.X;
                let offsetY = evt.clientY - this.startPos.Y;

                let targetX = this.startElePos.X + offsetX;
                let targetY = this.startElePos.Y + offsetY;

                TabMover.getInstance().TryMove(this, new Vector2D(targetX, targetY))
            }
        } else {
            this.endMoving()
        }
    }

    setStylePosition(positionStyle: string) {
        this.style.position = positionStyle
    }

    setScrPos(x: number, y: number) {
        this.setStylePosition("fixed")

        let computedStyle = getComputedStyle(this)

        let transformMatrix = new DOMMatrix()
        let currentElement: HTMLElement = this
        while(currentElement){
            let computedStyle = getComputedStyle(currentElement)
            transformMatrix = transformMatrix.multiply(new DOMMatrix(computedStyle.transform))
            currentElement = currentElement.parentElement
        }

        let transformedPoint = new DOMPoint(x, y).matrixTransform(transformMatrix.inverse())

        this.style.left = transformedPoint.x + "px"
        this.style.top = transformedPoint.y + "px"
    }

    setMarginLeft(marginLeft: number) {
        this.style.marginLeft = marginLeft.toString() + "px"
    }

    mouseUp(evt: MouseEvent) {
        this.endMoving()
    }

    endMoving() {
        if (this.isMoving) {
            OccupiedTitleManager.getInstance().dropTitle(this)
        }

        this.startMoving = false
        this.isMoving = false
        document.onmousemove = null
        document.onmouseup = null
    }

    getParentPanel(): HHPanel {
        if (this.parentPanel == null) {
            let parentPanelCandidate = this.parentElement
            while (parentPanelCandidate != null && !(parentPanelCandidate instanceof HHPanel)) {
                parentPanelCandidate = parentPanelCandidate.parentElement
            }
            if (parentPanelCandidate == null)
                throw "This title is not inside a panel??"
            this.parentPanel = parentPanelCandidate as HHPanel
        }
        return this.parentPanel
    }

    setTabIndex(newIdx: number) {
        this.setAttribute("tabindex", newIdx.toString())
    }
}

export {HHTitle}