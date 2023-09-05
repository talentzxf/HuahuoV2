import * as React from "react";
import {CSSUtils} from "../Utilities/CSSUtils";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {PropertySheet} from "hhcommoncomponents";
import {GetPropertyReactGenerator} from "./BasePropertyDivGeneratorX";

function getBtnClz() {
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
    }

    onItemSelected(propertySheet: PropertySheet, targetObj: any) {
        this.props?.openPanel()
        this.state.selectedObject = targetObj

        this.setState(this.state)
    }

    componentDidMount() {
        IDEEventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))

        this.props?.closePanel()
    }

    createButtonGroup() {
        return (
            <div id="buttons" className="inline-flex rounded-md shadow-sm divide-x divide-gray-300">
                <ToggleButton trueStateName={i18n.t("inspector.CollapseAll")}
                              falseStateName={i18n.t("inspector.OpenAll")}></ToggleButton>
                {
                    this.state?.selectedObject?.addComponent &&
                    <button className={getBtnClz()}> {i18n.t("inspector.AddComponent")}</button>
                }
                {
                    this.state?.selectedObject?.saveAsKeyFrame &&
                    <button className={getBtnClz()}> {i18n.t("inspector.SaveAsKeyFrame")} </button>
                }
                <ToggleButton trueStateName={i18n.t("inspector.LockObject")}
                              falseStateName={i18n.t("inspector.UnlockObject")}></ToggleButton>
                <ToggleButton trueStateName={i18n.t("inspector.ShowCenterSelector")}
                              falseStateName={i18n.t("inspector.HideCenterSelector")}></ToggleButton>
            </div>
        )
    }

    createComponentGroup() {
        let componentElements = []
        let propertySheet = this.state?.selectedObject?.getPropertySheet()
        if (propertySheet == null) {
            this.props.closePanel()
            return null
        }


        let index = 0

        for (let property of propertySheet.getProperties()) {
            if (property.hide)
                continue

            let generator = GetPropertyReactGenerator(property.type)
            if (generator) {
                let reactElement = React.createElement(generator, {
                    key: index++,
                    property: property
                })

                componentElements.push(reactElement)
            }
        }

        return (
            <div className="flex flex-col">
                {componentElements}
            </div>
        )
    }

    render() {
        return (
            <div className="w-full overflow-auto resize">
                {this.createButtonGroup()}
                {this.createComponentGroup()}
            </div>
        )
    }
}

export {InspectorX}