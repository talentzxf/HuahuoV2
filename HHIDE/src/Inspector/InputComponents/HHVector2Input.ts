import {CustomElement} from "hhcommoncomponents";
import {HHFloatInput} from "./HHFloatInput";
import {Func} from "mocha";

@CustomElement({
    selector: "hh-vector2-inputs"
})
class HHVector2Input extends HTMLElement implements RefreshableComponent {
    getter: Function
    setter: Function

    inputX: HHFloatInput
    inputY: HHFloatInput

    xKeyFrameCurveGetter: Function = null
    yKeyFrameCurveGetter: Function = null

    constructor(getter, setter, keyFrameCurveGetters: Function[] = null) {
        super();

        this.getter = getter
        this.setter = setter

        if (keyFrameCurveGetters?.length == 2) {
            this.xKeyFrameCurveGetter = keyFrameCurveGetters[0]
            this.yKeyFrameCurveGetter = keyFrameCurveGetters[1]
        }

        let _this = this
        this.inputX = new HHFloatInput(
            () => {
                return _this.getter().x
            },
            (xVal) => {
                _this.setter({
                    x: xVal,
                    y: Number(_this.inputY.value)
                })
            },
            this.xKeyFrameCurveGetter)

        this.inputY = new HHFloatInput(
            () => {
                return _this.getter().y
            },
            (yVal) => {
                _this.setter({
                    x: Number(_this.inputX.value),
                    y: yVal
                })
            },
            this.yKeyFrameCurveGetter)

        this.appendChild(this.inputX)
        this.appendChild(this.inputY)
    }

    set value(val) {
        this.inputX.value = val.x
        this.inputY.value = val.y
    }

    refresh() {
        this.value = this.getter()
    }
}

export {HHVector2Input}