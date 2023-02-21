import {CustomElement} from "hhcommoncomponents";
import {HHFloatInput} from "./HHFloatInput";

@CustomElement({
    selector: "hh-vector3-inputs"
})
class HHVector3Input extends HTMLElement implements RefreshableComponent {
    getter: Function
    setter: Function

    inputX: HHFloatInput
    inputY: HHFloatInput

    inputZ: HHFloatInput

    constructor(getter, setter) {
        super();

        this.getter = getter
        this.setter = setter

        let _this = this
        this.inputX = new HHFloatInput(
            () => {
                return _this.getter().x
            },
            (xVal) => {
                _this.setter({
                    x: xVal,
                    y: Number(_this.inputY.value),
                    z: Number(_this.inputZ.value)
                })
            })

        this.inputY = new HHFloatInput(
            () => {
                return _this.getter().y
            },
            (yVal) => {
                _this.setter({
                    x: Number(_this.inputX.value),
                    y: yVal,
                    z: Number(_this.inputZ.value),
                })
            })

        this.inputZ = new HHFloatInput(
            () => {
                return _this.getter().z
            },
            (yVal) => {
                _this.setter({
                    x: Number(_this.inputX.value),
                    y: yVal,
                    z: Number(_this.inputZ.value),
                })
            })

        this.appendChild(this.inputX)
        this.appendChild(this.inputY)
        this.appendChild(this.inputZ)
    }

    set value(val) {
        this.inputX.value = val.x
        this.inputY.value = val.y
        this.inputZ.value = val.z
    }

    refresh() {
        this.value = this.getter()
    }
}

export {HHVector3Input}