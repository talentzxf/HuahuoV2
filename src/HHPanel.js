const panelTemplateName = "HHPanel_Template"

let template = document.getElementById(panelTemplateName)
if(!template){
    const template = document.createElement("template");
    template.setAttribute("id", panelTemplateName)
    template.innerHTML = '<slot> Default Slots! </slot>'
}


export class HHPanel extends HTMLElement {
    constructor() {
        super();
        this._onSlotChange = this._onSlotChange.bind(this);
        this.attachShadow({mode:'open'})
        this.shadowRoot.appendChild(template.content.cloneNode(true))

        this._slots = this.shadowRoot.querySelector('slot');
        this._slots.addEventListener('slotchange', this._onSlotChange);
    }

    connectedCallback() {
        // Promise.all([
        //     customElements.whenDefined('howto-tab'),
        //     customElements.whenDefined('howto-panel'),
        // ]).then(_ => this._linkPanels());

        this._linkPanels()
    }

    disconnectedCallback(){
    }

    _onSlotChange() {
        this._linkPanels();
    }

    _linkPanels() {

        const tabs = this._allTabs();

    }

}

customElements.define('howto-tabs', HowtoTabs);

let howtoTabCounter = 0;

window.customElements.define('hh-panel', HHPanel)