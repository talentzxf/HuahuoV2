interface CustomElementConfig {
    selector:string;
    template?: string;
    style?: string;
    useShadow?: boolean;
}
const validateSelector = (selector: string) => {
    if (selector.indexOf('-') <= 0) {
        throw new Error('You need at least 1 dash in the custom element name!');
    }
};
const CustomElement = (config: CustomElementConfig) => (cls:any) => {
    if(window.customElements.get(config.selector))
        return;

    validateSelector(config.selector);
    // if (!config.template) {
    //     // throw new Error('You need to pass a template for the element');
    //     console.log("Template is null!")
    // }
    const template = document.createElement('template');
    if (config.style) {
        if(config.template)
            config.template = `<style>${config.style}</style> ${config.template}`;
        else
            config.template = `<style>${config.style}</style>`
    }
    template.innerHTML = config.template?config.template:"";

    const connectedCallback = cls.prototype.connectedCallback || function () {};
    const disconnectedCallback = cls.prototype.disconnectedCallback || function () {};

    cls.selector = config.selector
    cls.prototype.connectedCallback = function() {
        const clone = document.importNode(template.content, true);
        if (config.useShadow) {
            this.attachShadow({mode: 'open'}).appendChild(clone);
        } else {
            this.appendChild(clone);
        }

        if (this.componentWillMount) {
            this.componentWillMount();
        }
        connectedCallback.call(this);
        if (this.componentDidMount) {
            this.componentDidMount();
        }
    };

    cls.prototype.disconnectedCallback = function() {
        if (this.componentWillUnmount) {
            this.componentWillUnmount();
        }
        disconnectedCallback.call(this);
        if (this.componentDidUnmount) {
            this.componentDidUnmount();
        }
    };

    window.customElements.define(config.selector, cls);
};

export {CustomElement}