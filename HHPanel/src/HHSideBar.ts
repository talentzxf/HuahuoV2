import {CustomElement} from "hhcommoncomponents";
import {HHTitle} from "./HHTitle";
import {HHContent} from "./HHContent";
import {Vector2D} from "./math/Vector2D";
import {TabMoveParam, TabMover} from "./draggable/TabMover";
import {OccupiedTitleManager} from "./draggable/OccupiedTitleManager";
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
class HHSideBar extends HTMLElement {
    titleBar: HTMLDivElement

    isMoving: boolean = false
    startMoving: boolean = false
    startPos: Vector2D
    startElePos: Vector2D
    tabMover: SideBarTabMover = new SideBarTabMover()

    connectedCallback() {
        let content = this.querySelector("hh-sidebar-content") as HHContent
        let title = content.getAttribute("title") || "No Title"

        this.titleBar = document.createElement("div") as HTMLDivElement
        this.titleBar.innerHTML = title
        this.insertBefore(this.titleBar, content)
        this.titleBar.style.background = "lightgray"
        this.titleBar.style.userSelect = "none"
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

        let dockables: NodeListOf<HTMLElement> = document.querySelectorAll(".dockable")

        for (let dockable of dockables) {
            this.tabMover.AddFront(this.TryDock(dockable as HTMLElement))
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
                return true;
            } // Dock at bottom
            else if (candidatePos.Y + target.offsetHeight > clientRect.bottom - DOCKABLEMARGIN) {
                target.setScrPos(candidatePos.X, clientRect.bottom - target.offsetHeight)
                return true;
            }
            // Dock at left.
            else if (candidatePos.X < clientRect.left + DOCKABLEMARGIN) { // Put the element at top, up
                target.setScrPos(clientRect.left, clientRect.top)
                return true;
            } // Dock at right
            else if (candidatePos.X + target.offsetWidth > clientRect.right - DOCKABLEMARGIN) {
                target.setScrPos(clientRect.right - target.offsetWidth, clientRect.top)

                console.log("Here here!!", clientRect.right - target.offsetWidth)
                return true;
            }

            console.log("Nothing!!!")

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

}

export {HHSideBar, SideBarContent}