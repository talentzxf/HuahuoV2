import * as React from "react";
import {CSSUtils} from "../Utilities/CSSUtils";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {PropertySheet} from "hhcommoncomponents";
import {Func} from "mocha";

function getBtnClz(){
    let btnClz = CSSUtils.getButtonClass("teal")
    btnClz += " p-2 first:rounded-l-lg last:rounded-r-lg"
    return btnClz
}

type ToggleButtonState = {
    btnName: string
    isTrueState: boolean
}

type ToggleButtonProps = {
    trueStateName: string
    falseStateName: string
    onTrueState?: Function
    onFalseState?: Function
}

class ToggleButton extends React.Component<ToggleButtonProps, ToggleButtonState> {
    state = {
        btnName: "Unknown State",
        isTrueState: true
    }

    constructor(props) {
        super(props);

        this.state.btnName = this.props.trueStateName
    }

    onClick(e: Event) {
        this.state.isTrueState = !this.state.isTrueState
        if (this.state.isTrueState) {
            this.state.btnName = this.props.trueStateName
            this.props.onTrueState && this.props?.onTrueState()
        } else {
            this.state.btnName = this.props.falseStateName
            this.props.onFalseState && this.props?.onFalseState()
        }

        this.setState(this.state)
    }

    render() {
        return (
            <button className={getBtnClz()}
                    onClick={this.onClick.bind(this)}>
                {this.state.btnName}
            </button>
        )
    }
}

type InspectorProps = {
    closePanel: Function
    openPanel: Function
}

type InspectorState = {
    selectedObject: any
}

class InspectorX extends React.Component<InspectorProps, InspectorState> {
    state: InspectorState = {
        selectedObject: null,
    }

    constructor(props) {
        super(props);

        IDEEventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))

        this.props?.closePanel()
    }

    onItemSelected(propertySheet: PropertySheet, targetObj: any){
        this.props?.openPanel()
        this.state.selectedObject = targetObj

        this.setState(this.state)
    }

    createButtonGroup(){
        return (
            <div id="buttons" className="inline-flex rounded-md shadow-sm divide-x divide-gray-300">
                <ToggleButton trueStateName={i18n.t("inspector.CollapseAll")} falseStateName={i18n.t("inspector.OpenAll")}></ToggleButton>
                {
                    this.state?.selectedObject?.addComponent &&
                    <button className={getBtnClz()}>
                        Mount Component
                    </button>
                }
                {
                    this.state?.selectedObject?.saveAsKeyFrame &&
                    <button className={getBtnClz()}>
                        Save as keyframe
                    </button>
                }
                <ToggleButton trueStateName={i18n.t("inspector.LockObject")} falseStateName={i18n.t("inspector.UnlockObject")}></ToggleButton>
            </div>
        )
    }

    render() {
        return (
            <div className="w-full overflow-auto resize">
                {this.createButtonGroup()}
            </div>
        )
    }
}

export {InspectorX}