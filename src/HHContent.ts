import {CustomElement} from "./CustomComponent";


@CustomElement({
    selector: "hh-content",
    template:"<div></div>"
})
class HHContent extends HTMLElement {
    static contentCounter:number = 0;
    static get observedAttributes() {
        return ['selected'];
    }

    connectedCallback() {
        this.setAttribute('role', 'tab');
        if (!this.id)
            this.id = `hh-content-${HHContent.contentCounter++}`;

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

export {HHContent}