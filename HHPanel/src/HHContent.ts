import {CustomElement} from "hhcommoncomponents";


@CustomElement({
    selector: "hh-content",
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

        this.setAttribute('tabindex', String(-1));
        if(!this.style.height)
            this.style.height = 'fit-content'
    }

    attributeChangedCallback() {
        const value = this.hasAttribute('selected');
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