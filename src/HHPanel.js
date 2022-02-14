import "./HHContent"
// howto-panel {
//     padding: 20px;
//     background-color: lightgray;
// }

// howto-tabs:not(:defined), howto-tab:not(:defined), howto-panel:not(:defined) {
//     display: block;
// }

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
        
        .title_tabs span{
            padding: 5px
        }
        
        .title_tabs span[selected='true'] {
            background-color: bisque;
        }
        
        .panel_contents {
            border: 1px solid gray;
        }
    </style>
    <div class="title_tabs">
    </div>
    <div class="panel_contents" style="flex-basis: 100%;">
    </div>
    <slot></slot>
  `;
}

class HHPanel extends HTMLElement {
    constructor() {
        super();
        // this._onSlotChange = this._onSlotChange.bind(this);
        this.attachShadow({mode: 'open'});
        this.shadowRoot.appendChild(template.content.cloneNode(true));
        this._contentNodes = this.querySelectorAll('hh-content');
        this._tabs = this.shadowRoot.querySelector('.title_tabs');
        this._contents = this.shadowRoot.querySelector('.panel_contents')

        let _titleMap = new Map();
        let _this = this

        let tabIndex = 0
        this._contentNodes.forEach(
            node => {
                let title = node.getAttribute('title') || 'No Title'
                _titleMap.set(title, node)

                let titleSpan = document.createElement('span')
                titleSpan.innerHTML = title
                _this._tabs.appendChild(titleSpan)

                this._contents.appendChild(node)

                if(tabIndex == 0){
                    titleSpan.setAttribute('selected', true)
                    node.selected = true
                }else{
                    node.selected = false
                }

                node.setAttribute('tabindex', tabIndex++);
            }
        )

        // this._tabSlot.addEventListener('slotchange', this._onSlotChange);
        // this._panelSlot.addEventListener('slotchange', this._onSlotChange);
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
