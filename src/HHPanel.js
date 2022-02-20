import "./HHContent"
import {Vector2D} from "./math/Vector2D"

const panelTemplateName = "HHPanel_Template"

const KEYCODE = {
    DOWN: 40,
    LEFT: 37,
    RIGHT: 39,
    UP: 38,
    HOME: 36,
    END: 35,
};

let template = document.getElementById(panelTemplateName)
if (!template) {
    template = document.createElement('template');
    template.innerHTML = `
    <style>
        :host {
            display: flex;
            flex-direction: column;
            height: 100%;
            align-content: baseline;
        }
        
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
    </style>
    <div class="title_tabs">
    </div>
    <div class="panel_contents" style="flex-basis: 100%;">
    </div>
    <slot></slot>
  `;
}

class HHTitle extends HTMLElement {
    static get observedAttributes() {
        return ['tabindex']
    }

    constructor() {
        super();
        this.addEventListener("mousedown", this.mouseDown)
        this.addEventListener("mousemove", this.mouseMove)
        this.addEventListener("mouseup", this.mouseUp)

        this.startMoving = false
        this.isMoving = false
        this.startElePos = new Vector2D()
    }

    mouseDown(evt) {
        this.startPos = new Vector2D(evt.clientX, evt.clientY)
        this.startMoving = true
        this.isMoving = false
        console.log("Start:" + this.startPos.X + "," + this.startPos.Y)
        this.startElePos = new Vector2D(this.offsetLeft, this.offsetTop);
    }

    mouseMove(evt) {
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

                this.style.position = "absolute"
                this.style.left = targetX + "px"
                this.style.top = targetY + "px"
            }
        } else {
            this.endMoving()
        }
    }

    mouseUp(evt) {
        this.endMoving()
    }

    endMoving() {
        this.startMoving = false
        this.isMoving = false
    }

    attributeChangedCallback(name, oldValue, newValue) {
        if (name == 'tabindex')
            this.tabindex = newValue
    }
}

customElements.define('hh-title', HHTitle);

class HHPanel extends HTMLElement {
    constructor() {
        super();
        // this._onSlotChange = this._onSlotChange.bind(this);
        this.attachShadow({mode: 'open'});
        this.shadowRoot.appendChild(template.content.cloneNode(true));
        this._contentNodes = this.querySelectorAll('hh-content');
        this._tabs = this.shadowRoot.querySelector('.title_tabs');
        this._contents = this.shadowRoot.querySelector('.panel_contents')

        let _this = this

        let tabIndex = 0
        this._contentNodes.forEach(
            node => {
                let title = node.getAttribute('title') || 'No Title'
                let titleSpan = document.createElement('hh-title', {content: node})
                titleSpan.innerHTML = title
                titleSpan.setAttribute('tabindex', tabIndex)
                titleSpan.addEventListener('click', function (evt) {
                    let idx = titleSpan.getAttribute('tabindex')
                    _this.selectTab(idx)
                })

                _this._tabs.appendChild(titleSpan)

                this._contents.appendChild(node)
                node.setAttribute('tabindex', tabIndex);

                tabIndex++;
            }
        )

        _this.selectTab(0)

        // this._tabSlot.addEventListener('slotchange', this._onSlotChange);
        // this._panelSlot.addEventListener('slotchange', this._onSlotChange);
    }

    selectTab(tabindex) {
        // if(tabIndex == 0){
        //     titleSpan.setAttribute('selected', 'true')
        //     node.selected = true
        // }else{
        //     titleSpan.setAttribute('selected', 'false')
        //     node.selected = false
        // }

        let selectedTab = this.shadowRoot.querySelector('hh-title[tabindex="' + tabindex + '"]')
        let selectedContent = this.shadowRoot.querySelector('hh-content[tabindex="' + tabindex + '"]')
        selectedTab.setAttribute('selected', "true")
        selectedContent.selected = true

        let unselectedTabs = this.shadowRoot.querySelectorAll('hh-title:not([tabindex="' + tabindex + '"])')
        let unselectedContents = this.shadowRoot.querySelectorAll('hh-content:not([tabindex="' + tabindex + '"])')

        unselectedTabs.forEach(tab => {
            tab.setAttribute('selected', 'false')
        })

        unselectedContents.forEach(content => {
            content.selected = false
        })
    }

    //
    // connectedCallback() {
    //     this.addEventListener('keydown', this._onKeyDown);
    //     this.addEventListener('click', this._onClick);
    //
    //     if (!this.hasAttribute('role'))
    //         this.setAttribute('role', 'tablist');
    //
    //     Promise.all([
    //         customElements.whenDefined('howto-tab'),
    //         customElements.whenDefined('howto-panel'),
    //     ]).then(_ => this._linkPanels());
    // }
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

customElements.define('hh-panel', HHPanel);

export {HHPanel}
