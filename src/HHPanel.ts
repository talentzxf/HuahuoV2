import "./HHContent"
import {Rect2D} from "./math/Rect2D";
import {TabMover, TabMoveParam} from "./draggable/TabMover";
import {OccupiedTitleManager} from "./draggable/OccupiedTitleManager";
import {HHTitle} from "./HHTitle";
import {HHContent} from "./HHContent";
import {CustomElement} from "./CustomComponent";

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
    private _contentNodes: NodeListOf<HTMLElement>
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
                        console.log("Overlapping:")
                        console.log(titleBar)
                    }
                }

                // Didn't overlap with any child, it's in the right most
                if(!overlapWithChild){
                    OccupiedTitleManager.getInstance().setIsRightMost()
                }
            })

            return true;
        }
        console.log("Didn't overlap")

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

        let selectedTab = this.querySelector('hh-title[tabindex="' + tabindex + '"]')
        let selectedContent = this.querySelector('hh-content[tabindex="' + tabindex + '"]') as HHContent
        selectedTab.setAttribute('selected', "true")
        selectedContent.selected = true

        let unselectedTabs = this.querySelectorAll('hh-title:not([tabindex="' + tabindex + '"])') as NodeListOf<HHContent>
        let unselectedContents = this.querySelectorAll('hh-content:not([tabindex="' + tabindex + '"])') as NodeListOf<HHContent>

        unselectedTabs.forEach(tab => {
            tab.setAttribute('selected', 'false')
        })

        unselectedContents.forEach(content => {
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
                let titleSpan = document.createElement('hh-title')
                titleSpan.appendChild(node)
                titleSpan.innerHTML = title
                titleSpan.setAttribute('tabindex', tabIndex.toString())
                titleSpan.addEventListener('click', function (evt) {
                    let idx = Number(titleSpan.getAttribute('tabindex'))
                    _this.selectTab(idx)
                })

                _this._tabs.appendChild(titleSpan)

                this._contents.appendChild(node)
                node.setAttribute('tabindex', tabIndex.toString());

                tabIndex++;
            }
        )

        _this.selectTab(0)

        TabMover.getInstance().AddFront(this.onTitleMoving.bind(this))
    }

    //
    // disconnectedCallback() {
    //     this.removeEventListener('keydown', this._onKeyDown);
    //     this.removeEventListener('click', this._onClick);
    // }
    //
    // _onSlotChange() {
    //     this._linkPanels();
    // }
    //
    // _linkPanels() {
    //     const tabs = this._allTabs();
    //     tabs.forEach(tab => {
    //         const panel = tab.nextElementSibling;
    //         if (panel.tagName.toLowerCase() !== 'howto-panel') {
    //             console.error(`Tab #${tab.id} is not a` +
    //                 `sibling of a <howto-panel>`);
    //             return;
    //         }
    //
    //         tab.setAttribute('aria-controls', panel.id);
    //         panel.setAttribute('aria-labelledby', tab.id);
    //     });
    //
    //     const selectedTab =
    //         tabs.find(tab => tab.selected) || tabs[0];
    //     this._selectTab(selectedTab);
    // }
    //
    // _allPanels() {
    //     return Array.from(this.querySelectorAll('howto-panel'));
    // }
    //
    // _allTabs() {
    //     return Array.from(this.querySelectorAll('howto-tab'));
    // }
    //
    // _panelForTab(tab) {
    //     const panelId = tab.getAttribute('aria-controls');
    //     return this.querySelector(`#${panelId}`);
    // }
    //
    // _prevTab() {
    //     const tabs = this._allTabs();
    //     let newIdx = tabs.findIndex(tab => tab.selected) - 1;
    //
    //     return tabs[(newIdx + tabs.length) % tabs.length];
    // }
    //
    // _firstTab() {
    //     const tabs = this._allTabs();
    //     return tabs[0];
    // }
    //
    // _lastTab() {
    //     const tabs = this._allTabs();
    //     return tabs[tabs.length - 1];
    // }
    //
    // _nextTab() {
    //     const tabs = this._allTabs();
    //     let newIdx = tabs.findIndex(tab => tab.selected) + 1;
    //     return tabs[newIdx % tabs.length];
    // }
    //
    // reset() {
    //     const tabs = this._allTabs();
    //     const panels = this._allPanels();
    //
    //     tabs.forEach(tab => tab.selected = false);
    //     panels.forEach(panel => panel.hidden = true);
    // }
    //
    // _selectTab(newTab) {
    //     this.reset();
    //     const newPanel = this._panelForTab(newTab);
    //
    //     if (!newPanel)
    //         throw new Error(`No panel with id ${newPanelId}`);
    //     newTab.selected = true;
    //     newPanel.hidden = false;
    //     newTab.focus();
    // }
    //
    // _onKeyDown(event) {
    //     if (event.target.getAttribute('role') !== 'tab')
    //         return;
    //     if (event.altKey)
    //         return;
    //
    //     let newTab;
    //     switch (event.keyCode) {
    //         case KEYCODE.LEFT:
    //         case KEYCODE.UP:
    //             newTab = this._prevTab();
    //             break;
    //
    //         case KEYCODE.RIGHT:
    //         case KEYCODE.DOWN:
    //             newTab = this._nextTab();
    //             break;
    //
    //         case KEYCODE.HOME:
    //             newTab = this._firstTab();
    //             break;
    //
    //         case KEYCODE.END:
    //             newTab = this._lastTab();
    //             break;
    //
    //         default:
    //             return;
    //     }
    //
    //     event.preventDefault();
    //     this._selectTab(newTab);
    // }
    //
    // _onClick(event) {
    //     if (event.target.getAttribute('role') !== 'tab')
    //         return;
    //
    //     this._selectTab(event.target);
    // }
}

export {HHPanel}
