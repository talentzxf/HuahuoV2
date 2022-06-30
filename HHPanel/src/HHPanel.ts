import "./HHContent"
import {Rect2D} from "./math/Rect2D";
import {TabMover, TabMoveParam} from "./draggable/TabMover";
import {OccupiedTitleManager, SplitPanelDir} from "./draggable/OccupiedTitleManager";
import {HHTitle} from "./HHTitle";
import {HHContent} from "./HHContent";
import {CustomElement} from "hhcommoncomponents";
import {Vector2D} from "./math/Vector2D";
import {ShadowPanelManager} from "./draggable/ShadowPanelManager";

enum PanelEventNames{
    CONTENTSELECTED = "ContentSelected"
}

@CustomElement({
    selector: 'hh-panel',
    template: `<template>
        <div style="display: flex; flex-direction: column; height: 100%; width: 100%">
            <div class="title_tabs">
            </div>
            <div class="panel_contents" style="flex-basis: 100%;">
            </div>
        </div>
    </template>`,
    style: `        
        .title_tabs{
            padding:5px
        }
        
        .title_tabs hh-title{
            padding: 5px;
            cursor: pointer;
        }
        
        .title_tabs hh-title[selected='true'] {
            background-color: gray;
            border-bottom: 3px blue solid;
        }
        
        .title_tabs hh-title[selected='false'] {
            background-color: darkgray;
        }
        
        .panel_contents {
            border: 1px solid gray;
        }
        .title_tabs hh-title{
           -ms-user-select:none;
           -moz-user-select:none;
           -webkit-user-select:none;
           -webkit-touch-callout: none;
           -khtml-user-select: none;
            user-select:none;
        }
    `
})
class HHPanel extends HTMLElement {
    private _contentNodes: NodeListOf<HHContent>
    private _tabs: HTMLElement
    private _contents: HTMLElement
    private mInited:Boolean = false
    private maxTabId: number = 0

    constructor() {
        super();
    }

    handleTitleBar(ele: HHTitle, targetPos: Vector2D) {
        let tabs = this._tabs;

        let targetRect = new Rect2D(targetPos.X, targetPos.Y, targetPos.X + ele.offsetWidth, targetPos.Y + ele.offsetHeight);
        let titleBarRect = Rect2D.fromDomRect(tabs.getBoundingClientRect())

        if (titleBarRect.overlap(targetRect)) {
            ele.setScrPos(targetPos.X, tabs.offsetTop)

            // Check clip with other tabs in the tab group
            let overlapWithChild = false
            let titles = this._tabs.querySelectorAll('hh-title')
            titles.forEach(titleBar => {
                let childTitleBarRect = Rect2D.fromDomRect(titleBar.getBoundingClientRect())
                if (ele != titleBar) {
                    if (childTitleBarRect.overlap(targetRect)) {
                        overlapWithChild = true

                        OccupiedTitleManager.getInstance().setCandidate(titleBar as HHTitle, this, Number(ele.offsetWidth))
                    }
                }

                // Didn't overlap with any child, it's in the right most
                if (!overlapWithChild) {
                    OccupiedTitleManager.getInstance().setIsRightMost(this)
                }
            })

            ShadowPanelManager.getInstance().hideShadowPanel()
            return true;
        }
        return false;
    }

    handleContents(ele: HHTitle, targetPos: Vector2D) {

        // This is the last of the parent title group and is intersecing with this panel.
        if (ele.parentElement.querySelectorAll("hh-title").length <= 1
            && this == ele.getParentPanel())
            return;
        let contents = this._contents
        let contentRect = Rect2D.fromDomRect(contents.getBoundingClientRect())
        let targetRect = new Rect2D(targetPos.X, targetPos.Y, targetPos.X + ele.offsetWidth, targetPos.Y + ele.offsetHeight);

        let contentLU: Vector2D = contentRect.getLeftUp()
        let contentRD: Vector2D = contentRect.getRightDown()
        let shadowWidth = contentRect.width / 2
        let shadowHeight = contentRect.height / 2
        if (contentRect.overlap(targetRect)) {
            let shadowPanelRect: Rect2D

            if ((targetPos.X - contentLU.X) < contentRect.width * ShadowPanelManager.Bar) {  // LEFT
                shadowPanelRect = new Rect2D(contentLU.X, contentLU.Y,
                    contentLU.X + shadowWidth, contentRD.Y);
                OccupiedTitleManager.getInstance().setShadowCandidate(this, SplitPanelDir.LEFT)
            } else if (contentRD.X - targetPos.X < contentRect.width * ShadowPanelManager.Bar) {  // RIGHT
                shadowPanelRect = new Rect2D(contentLU.X + shadowWidth, contentLU.Y,
                    contentRD.X, contentRD.Y)
                OccupiedTitleManager.getInstance().setShadowCandidate(this, SplitPanelDir.RIGHT)
            } else if (targetPos.Y - contentLU.Y < contentRect.height * ShadowPanelManager.Bar) {  // UP
                shadowPanelRect = new Rect2D(contentLU.X, contentLU.Y, contentRD.X, contentLU.Y + shadowHeight)
                OccupiedTitleManager.getInstance().setShadowCandidate(this, SplitPanelDir.UP)
            } else if (contentRD.Y - targetPos.Y < contentRect.height * ShadowPanelManager.Bar) {  // DOWN
                shadowPanelRect = new Rect2D(contentLU.X, contentLU.Y + shadowHeight, contentRD.X, contentRD.Y)
                OccupiedTitleManager.getInstance().setShadowCandidate(this, SplitPanelDir.DOWN)
            } else {
                shadowPanelRect = new Rect2D(contentLU.X, contentLU.Y,
                    contentRD.X, contentRD.Y)

                OccupiedTitleManager.getInstance().setIsRightMost(this);
            }

            if (shadowPanelRect)
                ShadowPanelManager.getInstance().updateShadowPanel(shadowPanelRect)
        }

        return false
    }

    onTitleMoving(param: TabMoveParam) {
        let ele = param.ele as HHTitle;
        let targetPos = param.targetPos;
        if (this.handleTitleBar(ele, targetPos)
            || this.handleContents(ele, targetPos))
            return true;

        // Let other handlers handle this event.
        return false;
    }

    selectTab(tabindex: number) {
        // if(tabIndex == 0){
        //     titleSpan.setAttribute('selected', 'true')
        //     node.selected = true
        // }else{
        //     titleSpan.setAttribute('selected', 'false')
        //     node.selected = false
        // }

        let selectedTab = this.querySelector('hh-title[tabindex="' + tabindex + '"]') as HHTitle
        let selectedContent = selectedTab.getContent();
        selectedTab.setAttribute('selected', "true")
        selectedContent.selected = true

        let unselectedTabs = this.querySelectorAll('hh-title:not([tabindex="' + tabindex + '"])') as NodeListOf<HHTitle>

        unselectedTabs.forEach(tab => {
            let content = tab.getContent()
            tab.setAttribute('selected', 'false')
            content.selected = false
        })

        let customEvent = new CustomEvent(PanelEventNames.CONTENTSELECTED, {
            detail: {
                tabIndex: tabindex,
                content: selectedContent
            }
        })
        this.dispatchEvent(customEvent)
    }

    addContent(node: HHContent){
        let title = node.getAttribute('title') || 'No Title'
        let titleSpan = document.createElement('hh-title') as HHTitle
        titleSpan.appendChild(node)
        titleSpan.innerHTML = title
        titleSpan.setAttribute('tabindex', this.maxTabId.toString())
        titleSpan.addEventListener('click', function (evt) {
            let idx = Number(titleSpan.getAttribute('tabindex'))
            titleSpan.getParentPanel().selectTab(idx)
        })

        this._tabs.appendChild(titleSpan)
        titleSpan.setContent(node)
        titleSpan.setParentPanel(this)
        return this.maxTabId++;
    }

    initPanel() {
        /*
    :host {
    display: flex;
    flex-direction: column;
    height: 100%;
    align-content: baseline;
}
 */

        this.style.display = 'flex'
        this.style.flexDirection = this.parentElement.style.flexDirection
        this.style.height = '100%'
        this.style.width = '100%'
        this.style.alignContent = 'baseline'

        let template = this.querySelector('template');
        this.appendChild(template.content.cloneNode(true));

        this._contentNodes = this.querySelectorAll('hh-content');
        this._tabs = this.querySelector('.title_tabs');
        this._contents = this.querySelector('.panel_contents')

        if(this.getAttribute("content-style")){
            let style = this.getAttribute("content-style")
            this._contents.style.cssText = style
        }

        let _this = this

        this._contentNodes.forEach(
            node => {
                _this.addContent(node)
            }
        )

        if(_this._tabs.querySelector('hh-title'))
            _this.selectTab(0)

        TabMover.getInstance().AddFront(this.onTitleMoving.bind(this))

        // let nextSibling = DomHelper.getNextSiblingElementByName(this, ["hh-panel"])
        // if (nextSibling) {
        //     let splitter = document.createElement('hh-splitter')
        //     splitter.setAttribute("direction", this.parentElement.style.flexDirection)
        //     this.parentElement.insertBefore(splitter, nextSibling)
        // }
    }

    connectedCallback() {
        if(!this.mInited){
            this.initPanel()
            this.mInited = true
        }
    }

    getTitleCount(): number {
        return this._tabs.querySelectorAll("hh-title").length;
    }

    addChild(title: HHTitle) {
        title.setParentPanel(this)
    }

    getTabGroup(): HTMLElement {
        return this._tabs
    }

    getTitles(leftIdx: number, rightIdx: number): Array<HHTitle> {
        let resultArray: Array<HHTitle> = new Array();

        this._tabs.querySelectorAll("hh-title").forEach(title => {
            if (title instanceof HHTitle) {
                let titleEle = title as HHTitle
                if (titleEle.tabIndex >= leftIdx && titleEle.tabIndex <= rightIdx) {
                    resultArray.push(titleEle)
                }
            }
        })
        return resultArray;
    }

    renderTitles() {
        // Remove all tabs, sort and add again
        let allTitles: HHTitle[] = Array.from(this._tabs.querySelectorAll("hh-title"))

        while (this._tabs.firstChild) {
            this._tabs.removeChild(this._tabs.firstChild)
        }
        allTitles.sort(function elementCompare(a, b) {
            let idxA = a.tabIndex;
            let idxB = b.tabIndex;
            return idxA - idxB;
        })

        allTitles.forEach(title => {
            this._tabs.appendChild(title)
        })
    }

    getContentGroup() {
        return this._contents
    }
}

export {HHPanel, PanelEventNames}
