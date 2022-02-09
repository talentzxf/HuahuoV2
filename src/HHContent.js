export class HHContent extends HTMLElement {
    constructor() {
        super();
    }

    connectedCallback(){
        this.innerHTML = this.template
    }

    get template(){
        return `
        I'm the content now !!!
        `
    }
}

window.customElements.define('hh-content', HHContent)