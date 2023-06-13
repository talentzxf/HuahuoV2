import {CustomElement} from "hhcommoncomponents";
import {HHContent} from "./HHContent";
import {Vector2D} from "./math/Vector2D";
import {MovableElement, TabMoveParam, TabMover} from "./draggable/TabMover";
import {ChainCallback} from "./draggable/ResponsibleChain";

const DOCKABLEMARGIN = 20;

enum DIRECTIONS{
    LEFT = 0,
    RIGHT = 1,
    TOP = 2,
    BOTTOM = 3
}

@CustomElement({
    selector: "hh-sidebar-content",
})
class SideBarContent extends HTMLElement {

}

class SideBarTabMover extends TabMover {
    public constructor() { // Public the constructor.
        super();
    }
}

@CustomElement({
    selector: "hh-sidebar",
})
class HHSideBar extends HTMLElement implements MovableElement {
    currentlyDockedElement: HTMLElement = null;
    titleBar: HTMLDivElement
    content: HTMLElement

    minimizeButton: HTMLButtonElement = null;

    isMoving: boolean = false
    startMoving: boolean = false
    startPos: Vector2D
    startElePos: Vector2D
    tabMover: SideBarTabMover = new SideBarTabMover()

    registeredDockables: Set<HTMLElement> = new Set<HTMLElement>()

    allowedDirectionBoolean: boolean[] = [false, false, false, false] // left, right, top, bottom

    connectedCallback() {
        this.content = this.querySelector("hh-sidebar-content") as HHContent
        let title = this.content.getAttribute("title") || "No Title"

        // Remove the title to prevent unneeded tooltip.
        this.content.removeAttribute("title")

        this.titleBar = document.createElement("div") as HTMLDivElement

        let titleSpan = document.createElement("span")
        titleSpan.style.width = "100%"
        titleSpan.innerText = title
        this.titleBar.appendChild(titleSpan)

        this.insertBefore(this.titleBar, this.content)
        this.titleBar.style.display = "flex"
        this.titleBar.style.background = "lightgray"
        this.titleBar.style.userSelect = "none"
        this.titleBar.classList.add("title_tabs")
        this.titleBar.style.webkitUserSelect = "none"
        this.titleBar.style["ms-user-select"] = "none"
        this.titleBar.style["moz-user-select"] = "none"
        this.titleBar.style["webkit-touch-callout"] = "none"
        this.titleBar.style["khtml-user-select"] = "none"

        this.titleBar.classList.add("bg-info")

        this.style.position = "absolute"
        this.style.position = "absolute"
        this.style.background = "white"

        this.style.border = "1px solid black"

        this.style.zIndex = "1"

        this.titleBar.onmousedown = this.onTitleMouseDown.bind(this)

        this.refreshDockables()

        let allowedDockDirectionStr = this.getAttribute("allowedDockDirection")
        let allowedDockDirections = allowedDockDirectionStr == null ? []: allowedDockDirectionStr.split(",")
        if(allowedDockDirections.length == 0 || allowedDockDirections[0] == "all")
        {
            for(let idx:number = 0 ; idx < this.allowedDirectionBoolean.length; idx++){
                this.allowedDirectionBoolean[idx] = true
            }
        }

        for(let allowedDockDirection of allowedDockDirections){
            let dirNameUpper = allowedDockDirection.toUpperCase()
            let dirIdx = DIRECTIONS[dirNameUpper]
            this.allowedDirectionBoolean[dirIdx] = true
        }

        this.createTitleButtons()
    }

    createTitleButtons(){
        let titleToolBar = document.createElement("div")
        titleToolBar.style.display = "flex"
        titleToolBar.style.flexDirection = "row-reverse"

        this.minimizeButton = document.createElement("input")
        this.minimizeButton.className = "btn btn-outline-secondary btn-sm"
        this.minimizeButton.type = "button"
        this.minimizeButton.value = "-"

        this.minimizeButton.onclick = this.minimizeContent.bind(this)

        titleToolBar.appendChild(this.minimizeButton)
        this.titleBar.appendChild(titleToolBar)
    }

    isVisible(ele: HTMLElement) {
        return ele.offsetWidth > 0 && ele.offsetHeight > 0
    }

    maximizeContent(){
        this.minimizeButton.value = "-"
        this.content.style.display = "block"
        this.minimizeButton.onclick = this.minimizeContent.bind(this)
        this.refreshDockables()
    }

    minimizeContent(){
        this.minimizeButton.value = "+"
        this.content.style.display = "none"
        this.minimizeButton.onclick = this.maximizeContent.bind(this)

        this.refreshDockables()
    }

    refreshDockables() {
        let dockables: NodeListOf<HTMLElement> = document.querySelectorAll(".dockable")

        let firstDockable = null
        for (let dockable of dockables) {
            if (this.isVisible(dockable) && firstDockable == null)
                firstDockable = dockable

            if (!this.registeredDockables.has(dockable)) {
                this.tabMover.AddFront(this.TryDock(dockable as HTMLElement))
                this.registeredDockables.add(dockable)
            }
        }

        let initDockStatus = this.getAttribute("initDockStatus")
        if (initDockStatus && initDockStatus != "none" && firstDockable != null) {
            let clientRect: DOMRect = firstDockable.getBoundingClientRect()

            if (this.currentlyDockedElement == null || !this.isVisible(this.currentlyDockedElement)) {
                let dockStatusArray = initDockStatus.split("-")
                if (dockStatusArray.length == 2) {
                    let posX = clientRect[dockStatusArray[0]]
                    let posY = clientRect[dockStatusArray[1]]
                    if (dockStatusArray[0] == "right") {
                        posX -= this.offsetWidth
                    }

                    if (dockStatusArray[1] == "bottom") {
                        posY -= this.offsetHeight
                    }

                    this.setScrPos(posX, posY)
                }
            }
        }
    }

    hasOverlap(rect1: DOMRect, rect2: DOMRect){
        // Either one of the rectanges is a line
        if(rect1.width == 0 || rect1.height == 0 || rect2.width == 0 || rect2.height == 0)
            return false;

        // Either one is in the left of another one.
        if(rect1.x > rect2.x + rect2.width || rect1.x + rect1.width < rect2.x){
            return false
        }

        // Either one is on top of another
        if(rect1.y > rect2.y + rect2.height || rect1.y + rect1.height < rect2.y)
        {
            return false
        }

        return true
    }

    TryDock(dockable: HTMLElement): ChainCallback<TabMoveParam> {
        let _this = this
        return function (tabMoverParam: TabMoveParam) {
            let target = tabMoverParam.ele
            let candidatePos = tabMoverParam.targetPos
            if (!_this.isVisible(dockable))
                return false

            let clientRect: DOMRect = dockable.getBoundingClientRect()

            // Check if these two rects has overlap
            let targetRect: DOMRect = target.getBoundingClientRect()

            if(!_this.hasOverlap(clientRect, targetRect))
                return false

            // Dock at top
            if (_this.allowedDirectionBoolean[DIRECTIONS.TOP] && candidatePos.Y < clientRect.top + DOCKABLEMARGIN) {
                target.setScrPos(candidatePos.X, clientRect.top)
                target.currentlyDockedElement = dockable
                return true;
            } // Dock at bottom
            else if (_this.allowedDirectionBoolean[DIRECTIONS.BOTTOM] && candidatePos.Y + target.offsetHeight > clientRect.bottom - DOCKABLEMARGIN) {
                target.setScrPos(candidatePos.X, Math.max(clientRect.bottom - target.offsetHeight, 0))
                target.currentlyDockedElement = dockable
                return true;
            }
            // Dock at left.
            else if (_this.allowedDirectionBoolean[DIRECTIONS.LEFT] && candidatePos.X < clientRect.left + DOCKABLEMARGIN) { // Put the element at top, up
                target.setScrPos(clientRect.left, clientRect.top)
                target.currentlyDockedElement = dockable
                return true;
            } // Dock at right
            else if (_this.allowedDirectionBoolean[DIRECTIONS.RIGHT] && candidatePos.X + target.offsetWidth > clientRect.right - DOCKABLEMARGIN) {
                target.setScrPos(Math.max(clientRect.right - target.offsetWidth, 0), clientRect.top)
                target.currentlyDockedElement = dockable
                return true;
            } else {
                target.currentlyDockedElement = null
            }

            return false;
        }
    }

    // TODO: Avoid duplication with HHTitle
    onTitleMouseDown(evt: MouseEvent) {
        this.startPos = new Vector2D(evt.clientX, evt.clientY)
        this.startMoving = true
        this.isMoving = false
        this.startElePos = new Vector2D(this.offsetLeft, this.offsetTop);
        document.onmousemove = this.mouseMove.bind(this)
        document.onmouseup = this.mouseUp.bind(this)
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

                this.tabMover.TryMove(this, new Vector2D(targetX, targetY))

            }
        } else {
            this.endMoving()
        }
    }

    mouseUp(evt: MouseEvent) {
        this.endMoving()
    }

    endMoving() {
        this.startMoving = false
        this.isMoving = false
        document.onmousemove = null
        document.onmouseup = null
    }

    setScrPos(x, y) {
        this.style.left = x + "px"
        this.style.top = y + "px"
    }

    hide() {
        this.style.display = "none"
    }

    show() {
        this.style.display = "block"
        this.refreshDockables()
    }

}

export {HHSideBar, SideBarContent}