import "./HHContent"
import {Rect2D} from "./math/Rect2D";
import {TabMover, TabMoveParam} from "./draggable/TabMover";
import {OccupiedTitleManager} from "./draggable/OccupiedTitleManager";
import {HHTitle} from "./HHTitle";
import {HHContent} from "./HHContent";
import {CustomElement} from "./CustomComponent";
import {HSplitter} from "./HSplitter";

@CustomElement({
    selector: 'hh-panel',
    template: `<template>
        <div class="title_tabs">
        </div>
        <div class="panel_contents" style="flex-basis: 100%;">
        </div>
        <slot></slot>
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

    constructor() {
        super();
    }

    onTitleMoving(param: TabMoveParam) {
        let ele = param.ele as HHTitle;
        let targetPos = param.targetPos;
        let tabs = this._tabs;

        let targetRect = new Rect2D(targetPos.X, targetPos.Y, targetPos.X + ele.offsetWidth, targetPos.Y + ele.offsetHeight);
        let titleBarRect = Rect2D.fromDomRect(tabs.getBoundingClientRect())
        if (titleBarRect.overlap(targetRect)) {
            ele.setScrPos(param.targetPos.X, tabs.offsetTop)

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

            return true;
        }

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
    }

    connectedCallback() {
        /*
            :host {
            display: flex;
            flex-direction: column;
            height: 100%;
            align-content: baseline;
        }
         */

        this.style.display = 'flex'
        this.style.flexDirection = 'column'
        this.style.height = '100%'
        this.style.alignContent = 'baseline'

        let template = this.querySelector('template');
        this.appendChild(template.content.cloneNode(true));

        this._contentNodes = this.querySelectorAll('hh-content');
        this._tabs = this.querySelector('.title_tabs');
        this._contents = this.querySelector('.panel_contents')

        let _this = this

        let tabIndex = 0
        this._contentNodes.forEach(
            node => {
                let title = node.getAttribute('title') || 'No Title'
                let titleSpan = document.createElement('hh-title') as HHTitle
                titleSpan.appendChild(node)
                titleSpan.innerHTML = title
                titleSpan.setAttribute('tabindex', tabIndex.toString())
                titleSpan.addEventListener('click', function (evt) {
                    let idx = Number(titleSpan.getAttribute('tabindex'))
                    titleSpan.getParentPanel().selectTab(idx)
                })

                _this._tabs.appendChild(titleSpan)
                titleSpan.setContent(node)
                titleSpan.setParentPanel(_this)

                tabIndex++;
            }
        )

        _this.selectTab(0)

        TabMover.getInstance().AddFront(this.onTitleMoving.bind(this))

        let hsplitter = document.createElement('hh-hsplitter')
        this.appendChild(hsplitter)
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

export {HHPanel}
