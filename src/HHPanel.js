export class HHPanel extends HTMLElement{
    constructor() {
        super();
    }

    connectedCallback(){
        this.innerHTML = this.template
    }

    get template(){
        return `
        <div> Gekko Gekko !!!</div>
        `
    }
}

window.customElements.define('hh-panel', HHPanel)