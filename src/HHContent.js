
let howtoTabCounter = 0;

class HHContent extends HTMLElement {
    static get observedAttributes() {
        return ['selected'];
    }

    constructor() {
        super();
    }

    connectedCallback() {
        this.setAttribute('role', 'tab');
        if (!this.id)
            this.id = `howto-tab-generated-${howtoTabCounter++}`;

        this.setAttribute('aria-selected', 'false');
        this.setAttribute('tabindex', -1);
        this.style.height = 'fit-content'
        this._upgradeProperty('selected');
    }

    _upgradeProperty(prop) {
        if (this.hasOwnProperty(prop)) {
            let value = this[prop];
            delete this[prop];
            this[prop] = value;
        }
    }

    attributeChangedCallback() {
        const value = this.hasAttribute('selected');
        this.setAttribute('aria-selected', value);
        this.setAttribute('tabindex', value ? 0 : -1);
    }

    set selected(value) {
        value = Boolean(value);
        if (value)
            this.setAttribute('selected', '');
        else
            this.removeAttribute('selected');
    }

    get selected() {
        return this.hasAttribute('selected');
    }
}
customElements.define('hh-content', HHContent);

let howtoPanelCounter = 0;

class HowtoPanel extends HTMLElement {
    constructor() {
        super();
    }

    connectedCallback() {
        this.setAttribute('role', 'tabpanel');
        if (!this.id)
            this.id = `howto-panel-generated-${howtoPanelCounter++}`;
    }
}
customElements.define('howto-panel', HowtoPanel);