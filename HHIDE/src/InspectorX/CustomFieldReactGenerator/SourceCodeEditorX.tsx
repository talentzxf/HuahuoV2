import * as React from "react";
import {PropertyProps} from "../BasePropertyX";
import {CSSUtils} from "../../Utilities/CSSUtils";
import {formManager} from "../../Utilities/FormManager";
import {registerCustomFieldPropertyX} from "../../ComponentProxy/ComponentProxy";
import {SourceCodeFormX} from "../../UIComponents/SourceCodeFormX";
import {CodeComponent} from "hhenginejs";

class SourceCodeEditorX extends React.Component<PropertyProps, any> {
    render() {
        let targetComponent = this.props.property.config.targetComponent
        return (<button className={CSSUtils.getButtonClass("indigo")} type="button" onClick={() => {
            formManager.openReactForm(SourceCodeFormX, {
                targetComponent: targetComponent
            })
        }}>{i18n.t("edit")}</button>)
    }
}

registerCustomFieldPropertyX(CodeComponent.name, "editSourceCode", SourceCodeEditorX)

export {SourceCodeEditorX}