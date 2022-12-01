import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-int-array-input"
})
class HHIntArray extends HTMLElement implements RefreshableComponent{
    refresh() {
    }
}

export {HHIntArray}