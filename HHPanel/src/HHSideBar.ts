import {CustomElement} from "hhcommoncomponents";
import {HHContent} from "./HHContent";
import {Vector2D} from "./math/Vector2D";
import {MovableElement, TabMoveParam, TabMover} from "./draggable/TabMover";
import {ChainCallback} from "./draggable/ResponsibleChain";

const DOCKABLEMARGIN = 20;

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

    isMoving: boolean = false
    startMoving: boolean = false
    startPos: Vector2D
    startElePos: Vector2D
    tabMover: SideBarTabMover = new SideBarTabMover()

    registeredDockables: Set<HTMLElement> = new Set<HTMLElement>()

    connectedCallback() {
        let content = this.querySelector("hh-sidebar-content") as HHContent
        let title = content.getAttribute("title") || "No Title"

        this.titleBar = document.createElement("div") as HTMLDivElement
        this.titleBar.innerHTML = title
        this.insertBefore(this.titleBar, content)
        this.titleBar.style.background = "lightgray"
        this.titleBar.style.userSelect = "none"
        this.titleBar.classList.add("title_tabs")
        this.titleBar.style.webkitUserSelect = "none"
        this.titleBar.style["ms-user-select"] = "none"
        this.titleBar.style["moz-user-select"] = "none"
        this.titleBar.style["webkit-touch-callout"] = "none"
        this.titleBar.style["khtml-user-select"] = "none"

        this.style.position = "absolute"
        this.style.position = "absolute"
        this.style.background = "white"

        this.style.border = "1px solid black"

        this.style.zIndex = "1"

        this.titleBar.onmousedown = this.onTitleMouseDown.bind(this)

        this.refreshDockables()
    }

    refreshDockables() {
        let dockables: NodeListOf<HTMLElement> = document.querySelectorAll(".dockable")

        let firstDockable = null
        for (let dockable of dockables) {
            if (firstDockable == null)
                firstDockable = dockable

            if (!this.registeredDockables.has(dockable)) {
                this.tabMover.AddFront(this.TryDock(dockable as HTMLElement))
                this.registeredDockables.add(dockable)
            }
        }

        let initDockStatus = this.getAttribute("initDockStatus")
        if (initDockStatus && initDockStatus != "none" && firstDockable != null) {
            let clientRect: DOMRect = firstDockable.getBoundingClientRect()

            if (this.currentlyDockedElement == null) {
                let dockStatusArray = initDockStatus.split("-")
                if (dockStatusArray.length == 2) {
                    this.setScrPos(clientRect[dockStatusArray[0]], clientRect[dockStatusArray[1]])
                }
            } else {
                let dockStatusArray = initDockStatus.split("-")
                if (dockStatusArray.length == 2) {
                    this.setScrPos(clientRect[dockStatusArray[0]], clientRect[dockStatusArray[1]])
                }
            }
        }
    }

    TryDock(dockable: HTMLElement): ChainCallback<TabMoveParam> {
        return function (tabMoverParam: TabMoveParam) {
            let target = tabMoverParam.ele
            let candidatePos = tabMoverParam.targetPos

            let clientRect: DOMRect = dockable.getBoundingClientRect()

            // Dock at top
            if (candidatePos.Y < clientRect.top + DOCKABLEMARGIN) {
                target.setScrPos(candidatePos.X, clientRect.top)
                target.currentlyDockedElement = dockable
                return true;
            } // Dock at bottom
            else if (candidatePos.Y + target.offsetHeight > clientRect.bottom - DOCKABLEMARGIN) {
                target.setScrPos(candidatePos.X, clientRect.bottom - target.offsetHeight)
                target.currentlyDockedElement = dockable
                return true;
            }
            // Dock at left.
            else if (candidatePos.X < clientRect.left + DOCKABLEMARGIN) { // Put the element at top, up
                target.setScrPos(clientRect.left, clientRect.top)
                target.currentlyDockedElement = dockable
                return true;
            } // Dock at right
            else if (candidatePos.X + target.offsetWidth > clientRect.right - DOCKABLEMARGIN) {
                target.setScrPos(clientRect.right - target.offsetWidth, clientRect.top)
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

    show(){
        this.style.display = "block"
    }

}

export {HHSideBar, SideBarContent}