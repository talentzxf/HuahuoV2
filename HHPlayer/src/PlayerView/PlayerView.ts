import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-player"
})
class PlayerView extends HTMLElement {
    connectedCallback(){
        this.innerText = "PlayerView!!!"
    }
}

export {PlayerView}