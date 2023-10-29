import * as React from "react";
import {CSSUtils} from "../Utilities/CSSUtils";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {HHToast, PropertySheet, PropertyType} from "hhcommoncomponents";
import {GetPropertyReactGenerator} from "./BasePropertyX";
import {formManager} from "../Utilities/FormManager";
import {huahuoEngine} from "hhenginejs";
import {ComponentListFormX} from "./ComponentListFormX";
import "./PropertyTypes"
import {sceneViewManager} from "../SceneView/SceneViewManager";

function getBtnClz() {
    let btnClz = CSSUtils.getButtonClass("teal")
    btnClz += " p-2 first:rounded-l-lg last:rounded-r-lg border-solid border-2 border-sky-500"
    return btnClz
}

type ToggleButtonState = {
    isTrueState: boolean
}

type ToggleButtonProps = {
    trueStateName: string
    falseStateName: string
    onTrueState?: Function
    onFalseState?: Function
    getCurrentState?: Function
}

class ToggleButton extends React.Component<ToggleButtonProps, ToggleButtonState> {
    state = {
        isTrueState: true
    }

    constructor(props) {
        super(props);
    }

    onClick(e: Event) {
        this.state.isTrueState = !this.state.isTrueState
        if (this.state.isTrueState) {
            this.props.onTrueState && this.props?.onTrueState()
        } else {
            this.props.onFalseState && this.props?.onFalseState()
        }

        this.setState(this.state)
    }

    render() {

        if (this.props.getCurrentState) {
            this.state.isTrueState = this.props.getCurrentState()
        }

        return (
            <button className={getBtnClz()}
                    onClick={this.onClick.bind(this)}>
                {this.state.isTrueState ? this.props.trueStateName : this.props.falseStateName}
            </button>
        )
    }
}

type InspectorProps = {}

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
        this.state.selectedObject = targetObj
        this.state.property = property == null ? targetObj.getPropertySheet() : property

        this.setState(this.state)
    }

    unselectObjects() {
        setTimeout(() => {
            this.state.selectedObject = null
            this.state.property = sceneViewManager.getFocusedSceneView().getPropertySheet()

            this.setState(this.state)
        })
    }

    componentChanged(targetObj: any) {
        this.forceUpdate()
    }

    onPlayFrame() {
        this.forceUpdate()
    }

    objectDeleted(targetObj) {
        this.unselectObjects()
    }

    componentDidMount() {
        IDEEventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))
        IDEEventBus.getInstance().on(EventNames.UNSELECTOBJECTS, this.unselectObjects.bind(this))
        IDEEventBus.getInstance().on(EventNames.COMPONENTCHANGED, this.componentChanged.bind(this))
        IDEEventBus.getInstance().on(EventNames.PLAYFRAME, this.onPlayFrame.bind(this))
        IDEEventBus.getInstance().on(EventNames.OBJECTDELETED, this.objectDeleted.bind(this))

        this.unselectObjects()
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
        formManager.openReactForm(ComponentListFormX, {
            componentNames: componentNames,
            targetObject: this.state.selectedObject
        })
    }

    moreThan2Components() {
        if (this.state?.property == null)
            return false

        let totalComponentCount = 0;
        for (let property of this.state.property.getProperties()) {
            if (property.hide)
                continue

            if (property.type == PropertyType.COMPONENT)
                totalComponentCount++
        }

        return totalComponentCount >= 2
    }

    createButtonGroup() {
        let targetObject = this.state.selectedObject

        return (
            <div id="buttons" className="inline-flex rounded-md shadow-sm divide-x divide-gray-300 flex flex-col">
                <div className="flex flex-row">
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
                            targetObject.saveAsKeyFrame()
                            HHToast.info(i18n.t("toast.keyframeSaved"))
                        }}> {i18n.t("inspector.SaveAsKeyFrame")} </button>
                    }
                </div>
                <div className="flex flex-row">
                    {
                        this.state?.selectedObject?.isLocked &&
                        <ToggleButton trueStateName={i18n.t("inspector.LockObject")}
                                      falseStateName={i18n.t("inspector.UnlockObject")}
                                      onTrueState={() => {
                                          targetObject.setIsLocked(false)
                                      }}
                                      onFalseState={() => {
                                          targetObject.setIsLocked(true)
                                      }}
                                      getCurrentState={() => {
                                          return !targetObject.isLocked()
                                      }}></ToggleButton>
                    }
                    {
                        this.state?.selectedObject?.isShowCenterSelector &&
                        <ToggleButton trueStateName={i18n.t("inspector.ShowCenterSelector")}
                                      falseStateName={i18n.t("inspector.HideCenterSelector")}
                                      onTrueState={targetObject.hideCenterSelector}
                                      onFalseState={targetObject.showCenterSelector}
                                      getCurrentState={() => {
                                          return !targetObject.isShowCenterSelector()
                                      }}></ToggleButton>
                    }
                </div>
            </div>
        )
    }

    createComponentGroup() {
        let componentElements = []
        let propertySheet = this.state.property
        if (propertySheet == null) {
            this.unselectObjects()
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
            // <div className="w-full overflow-auto resize">
            <div className="w-full overflow-auto">
                {this.state.selectedObject == null && this.state.property == null &&
                    <span> Please select something to start working</span>}
                {this.state.selectedObject != null && this.createButtonGroup()}
                {this.createComponentGroup()}
            </div>
        )
    }
}

export {InspectorX}