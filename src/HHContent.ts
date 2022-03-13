
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
        this.setAttribute('tabindex', String(-1));
        this.style.height = 'fit-content'
    }

    attributeChangedCallback() {
        const value = this.hasAttribute('selected');
        this.setAttribute('aria-selected', String(value));
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