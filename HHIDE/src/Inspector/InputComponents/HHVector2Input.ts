import {CustomElement} from "hhcommoncomponents";
import {HHFloatInput} from "./HHFloatInput";

@CustomElement({
    selector: "hh-vector2-inputs"
})
class HHVector2Input extends HTMLElement implements RefreshableComponent{
    getter: Function
    setter: Function

    inputX : HHFloatInput
    inputY : HHFloatInput

    constructor(getter, setter) {
        super();

        this.getter = getter
        this.setter = setter

        let _this = this
        this.inputX = new HHFloatInput(
            ()=>{
                return _this.getter().x
            },
            (xVal)=>{
                _this.setter(xVal, _this.inputY.value)
            })

        this.inputY = new HHFloatInput(
            ()=>{
                return _this.getter().y
            },
            (yVal)=>{
                _this.setter(_this.inputX.value, yVal)
            })

        this.appendChild(this.inputX)
        this.appendChild(this.inputY)
    }

    set value(val){
        this.inputX.value = val.x
        this.inputY.value = val.y
    }

    refresh(){
        this.value = this.getter()
    }
}

export {HHVector2Input}