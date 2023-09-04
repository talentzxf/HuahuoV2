import * as React from "react";
import {CSSUtils} from "../Utilities/CSSUtils";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {PropertySheet} from "hhcommoncomponents";

function getBtnClz(){
    let btnClz = CSSUtils.getButtonClass("teal")
    btnClz += " p-2 first:rounded-l-lg last:rounded-r-lg"
    return btnClz
}

type CollapseAllBtnState = {
    btnName: string
    isCollapse: boolean
}

class CollapseAllBtn extends React.Component<any, CollapseAllBtnState> {
    state = {
        btnName: i18n.t("inspector.CollapseAll"),
        isCollapse: true
    }

    onClick(e: Event) {
        this.state.isCollapse = !this.state.isCollapse
        if (this.state.isCollapse) {
            this.state.btnName = i18n.t("inspector.CollapseAll")
        } else {
            this.state.btnName = i18n.t("inspector.OpenAll")
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
    }

    onItemSelected(propertySheet: PropertySheet, targetObj: any){
        this.state.selectedObject = targetObj

        this.setState(this.state)
    }

    render() {
        return (
            <div className="w-full overflow-auto resize">
                <div id="buttons" className="inline-flex rounded-md shadow-sm divide-x divide-gray-300">
                    <CollapseAllBtn></CollapseAllBtn>
                    {
                        this.state.selectedObject && this.state.selectedObject["addComponent"] &&
                        <button className={getBtnClz()}>
                            Mount Component
                        </button>
                    }
                    {
                        this.state.selectedObject && this.state.selectedObject["saveAsKeyFrame"] &&
                        <button className={getBtnClz()}>
                            Save as keyframe
                        </button>
                    }
                </div>
            </div>
        )
    }
}

export {InspectorX}