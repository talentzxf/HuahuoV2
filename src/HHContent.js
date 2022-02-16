
let contentCounter = 0;

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
            this.id = `hh-content-${contentCounter++}`;

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
    }

    set selected(value) {
        value = Boolean(value);
        if (value)
        {
            this.setAttribute('selected', 'true');
            this.hidden = false
        }
        else{
            this.setAttribute('selected', 'false');
            this.hidden = true
        }
    }

    get selected() {
        return this.hasAttribute('selected');
    }
}
customElements.define('hh-content', HHContent);