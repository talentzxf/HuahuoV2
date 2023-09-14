import * as React from "react"
import {registerCustomFieldPropertyX} from "../../ComponentProxy/ComponentProxy";
import {EventGraphComponent} from "hhenginejs";
import {formManager} from "../../Utilities/FormManager";
import {EventGraphForm} from "../../EventGraphUI/EventGraphForm";
import {PropertyProps} from "../BasePropertyX";
import {CSSUtils} from "../../Utilities/CSSUtils";

class EventGraphEditorX extends React.Component<PropertyProps, any> {
    render() {
        let targetComponent = this.props.property.config.targetComponent
        return (<button className={CSSUtils.getButtonClass("indigo")} type="button" onClick={() => {
            let eventGraphFrom = formManager.openForm(EventGraphForm)
            eventGraphFrom.setTargetComponent(targetComponent)
        }}>{i18n.t("edit")}</button>)
    }
}

registerCustomFieldPropertyX(EventGraphComponent.name, "eventGraph", EventGraphEditorX)

export {EventGraphEditorX}