import * as React from "react";
import {CSSUtils} from "../Utilities/CSSUtils";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {PropertySheet} from "hhcommoncomponents";
import {GetPropertyReactGenerator} from "./BasePropertyX";
import {formManager} from "../Utilities/FormManager";
import {huahuoEngine} from "hhenginejs";
import {ComponentListFormX} from "./ComponentListFormX";
import {HHToast} from "hhcommoncomponents";

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
    property: PropertySheet
}

class InspectorX extends React.Component<InspectorProps, InspectorState> {
    state: InspectorState = {
        selectedObject: null,
        property: null
    }

    onItemSelected(property: PropertySheet, targetObj: any) {
        this.props?.openPanel()
        this.state.selectedObject = targetObj
        this.state.property = property == null ? targetObj.getPropertySheet() : property

        this.setState(this.state)
    }

    unselectObjects() {
        this.props?.closePanel()
    }

    componentChanged(targetObj: any) {
        this.forceUpdate()
    }

    timelineCellClicked() {
        this.forceUpdate()
    }

    objectDeleted(targetObj) {
        this.props?.closePanel()
    }

    componentDidMount() {
        IDEEventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))
        IDEEventBus.getInstance().on(EventNames.UNSELECTOBJECTS, this.unselectObjects.bind(this))
        IDEEventBus.getInstance().on(EventNames.COMPONENTCHANGED, this.componentChanged.bind(this))
        IDEEventBus.getInstance().on(EventNames.CELLCLICKED, this.timelineCellClicked.bind(this))
        IDEEventBus.getInstance().on(EventNames.OBJECTDELETED, this.objectDeleted.bind(this))

        this.props?.closePanel()
    }

    componentElementRefs = []

    openAll() {
        for (let component of this.componentElementRefs) {
            if (component && component.openSection)
                component.openSection()
        }
    }

    closeAll() {
        for (let component of this.componentElementRefs) {
            if (component && component.closeSection)
                component.closeSection()
        }
    }

    addComponent() {
        let componentNames = huahuoEngine.getAllCompatibleComponents(this.state.selectedObject)
        formManager.openReactForm(ComponentListFormX, {componentNames: componentNames, targetObject: this.state.selectedObject})
    }

    moreThan2Components() {
        if (this.state?.property == null)
            return false

        let totalComponentCount = 0;
        for (let property of this.state.property.getProperties()) {
            if (property.hide)
                continue

            if (property.type == PropertySheet.COMPONENT)
                totalComponentCount++
        }

        return totalComponentCount > 2
    }

    createButtonGroup() {
        return (
            <div id="buttons" className="inline-flex rounded-md shadow-sm divide-x divide-gray-300">
                {
                    this.moreThan2Components() && <ToggleButton trueStateName={i18n.t("inspector.CollapseAll")}
                                                                falseStateName={i18n.t("inspector.OpenAll")}
                                                                onTrueState={this.openAll.bind(this)}
                                                                onFalseState={this.closeAll.bind(this)}></ToggleButton>
                }
                {
                    this.state?.selectedObject?.addComponent &&
                    <button className={getBtnClz()}
                            onClick={this.addComponent.bind(this)}> {i18n.t("inspector.AddComponent")}</button>
                }
                {
                    this.state?.selectedObject?.saveAsKeyFrame &&
                    <button className={getBtnClz()} onClick={() => {
                        this.state.selectedObject.saveAsKeyFrame()
                        HHToast.info(i18n.t("toast.keyframeSaved"))
                    }}> {i18n.t("inspector.SaveAsKeyFrame")} </button>
                }
                {
                    this.state?.selectedObject?.isLocked && <ToggleButton trueStateName={i18n.t("inspector.LockObject")}
                                                                          falseStateName={i18n.t("inspector.UnlockObject")}></ToggleButton>
                }
                {
                    this.state?.selectedObject?.isShowCenterSelector &&
                    <ToggleButton trueStateName={i18n.t("inspector.ShowCenterSelector")}
                                  falseStateName={i18n.t("inspector.HideCenterSelector")}></ToggleButton>
                }
            </div>
        )
    }

    createComponentGroup() {
        let componentElements = []
        let propertySheet = this.state.property
        if (propertySheet == null) {
            this.props.closePanel()
            return null
        }

        let index = 0

        this.componentElementRefs = []
        for (let property of propertySheet.getProperties()) {
            if (property.hide)
                continue

            let generator = GetPropertyReactGenerator(property.type)
            if (generator) {
                let reactElement = React.createElement(generator, {
                    key: index++,
                    property: property,
                    ref: (instance) => {
                        if (instance != null) // not sure why, but React will pass null instance here, Maybe because the component is deleted?
                            this.componentElementRefs.push(instance)
                    }
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