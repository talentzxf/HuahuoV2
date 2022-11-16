import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-color-stop-array-input"
})
class HHColorStopArrayInput extends HTMLElement implements RefreshableComponent{
    getter: Function
    setter: Function
    constructor(getter, setter) {
        super();
        
        this.getter = getter
        this.setter = setter
    }

    refresh() {
    }

}

export {HHColorStopArrayInput}
