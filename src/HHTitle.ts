import {Vector2D} from "./math/Vector2D";
import {TabMover} from "./draggable/TabMover";
import {CustomElement} from "./CustomComponent";
import {OccupiedTitleManager} from "./draggable/OccupiedTitleManager";
import {HHPanel} from "./HHPanel";

@CustomElement({
    selector:"hh-title",
    template:`<template></template>`,
})
class HHTitle extends HTMLElement {
    private startMoving: Boolean = false
    private isMoving: Boolean = false
    private startElePos: Vector2D = new Vector2D()
    private startPos: Vector2D
    private parentPanel: HHPanel

    static get tabIndex() {
        return ['tabindex']
    }

    constructor() {
        super();

        this.addEventListener("mousedown", this.mouseDown)
        this.addEventListener("mouseup", this.mouseUp)
    }

    setParentPanel(panel: HHPanel){
        this.parentPanel = panel
        panel.getTabGroup().appendChild(this)
    }

    mouseDown(evt:MouseEvent) {
        this.startPos = new Vector2D(evt.clientX, evt.clientY)
        this.startMoving = true
        this.isMoving = false
        console.log("Start:" + this.startPos.X + "," + this.startPos.Y)
        this.startElePos = new Vector2D(this.offsetLeft, this.offsetTop);
        document.onmousemove = this.mouseMove.bind(this)
    }

    connectedCallback() {
        console.log("Title connectedCallback")
    }

    mouseMove(evt:MouseEvent) {
        if (evt.buttons == 1) {
            if (this.startMoving && !this.startPos.equals(evt.clientX, evt.clientY)) {
                this.isMoving = true
            }

            if (this.isMoving) {
                console.log("IsMoving!!!")
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

    setStylePosition(positionStyle:string){
        this.style.position = positionStyle
    }

    setScrPos(x:number, y:number){
        this.setStylePosition("absolute")
        this.style.left = x + "px"
        this.style.top = y + "px"
    }

    setMarginLeft(marginLeft: number){
        this.style.marginLeft = marginLeft.toString() + "px"
    }

    mouseUp(evt:MouseEvent) {
        this.endMoving()
    }

    endMoving() {
        if(this.isMoving){
            OccupiedTitleManager.getInstance().dropTitle(this)
        }

        this.startMoving = false
        this.isMoving = false
        document.onmousemove = null
    }

    getParentPanel():HHPanel {
        if(this.parentPanel == null){
            let parentPanelCandidate = this.parentElement
            while(parentPanelCandidate != null && !(parentPanelCandidate instanceof HHPanel)){
                parentPanelCandidate = parentPanelCandidate.parentElement
            }
            if(parentPanelCandidate == null)
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