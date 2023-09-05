import {RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {vector2PropertyGenerator} from "./Vector2PropertyDivGenerator";
import {PropertyType} from "hhcommoncomponents"
import {floatPropertyDivGenerator} from "./FloatPropertyDivGenerator";
import {colorPropertyDivGenerator} from "./ColorPropertyDivGenerator";
import {stringPropertyDivGenerator} from "./StringPropertyDivGenerator";
import {buttonPropertyDivGenerator} from "./ButtonPropertyDivGenerator";
import {groupPropertyDivGenerator} from "./GroupPropertyDivGenerator";
import {referencePropertyDivGenerator} from "./ReferencePropertyDivGenerator";
import {componentPropertyDivGenerator, panelPropertyDivGenerator} from "./PanelPropertyDivGenerator";
import {arrayPropertyDivGenerator} from "./ArrayPropertyDivGenerator";
import {colorStopArrayPropertyDivGenerator} from "./ColorStopArrayPropertyDivGenerator";
import {booleanPropertyDivGenerator} from "./BooleanPropertyDivGenerator";
import {keyFramesPropertyDivGenerator} from "./KeyFramesPropertyDivGenerator";
import {subComponentsDivGenerator} from "./SubComponentsDivGenerator";
import {vector3PropertyGenerator} from "./Vector3PropertyDivGenerator";
import {customFieldDivGenerator} from "./CustomFieldDivGenerator";
import {RegisterReactGenerator} from "../InspectorX/BasePropertyDivGeneratorX";
import {ComponentPropertyGeneratorX, PanelPropertyReactGeneratorX,} from "../InspectorX/PanelPropertyReactGeneratorX";
import {StringPropertyGeneratorX} from "../InspectorX/StringPropertyGeneratorX";
import {Vector2PropertyReactGeneratorX} from "../InspectorX/Vector2PropertyReactGeneratorX";
import {FloatPropertyReactGeneratorX} from "../InspectorX/FloatPropertyReactGeneratorX";

// Avoid being imported twice.
// TODO: Any less dirty approach ??
if (!window["IsTypesRegistered"]) {
    RegisterDivGenerator(PropertyType.COLOR, colorPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.NUMBER, floatPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.BOOLEAN, booleanPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.VECTOR2, vector2PropertyGenerator)
    RegisterDivGenerator(PropertyType.VECTOR3, vector3PropertyGenerator)
    RegisterDivGenerator(PropertyType.STRING, stringPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.BUTTON, buttonPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.GROUP, groupPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.SHAPE, referencePropertyDivGenerator)
    RegisterDivGenerator(PropertyType.PANEL, panelPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.ARRAY, arrayPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.COMPONENT, componentPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.COLORSTOPARRAY, colorStopArrayPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.KEYFRAMES, keyFramesPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.SUBCOMPONENTARRAY, subComponentsDivGenerator)
    RegisterDivGenerator(PropertyType.CUSTOMFIELD, customFieldDivGenerator)

    RegisterReactGenerator(PropertyType.COMPONENT, ComponentPropertyGeneratorX)
    RegisterReactGenerator(PropertyType.PANEL, PanelPropertyReactGeneratorX)
    RegisterReactGenerator(PropertyType.STRING, StringPropertyGeneratorX)
    RegisterReactGenerator(PropertyType.NUMBER, FloatPropertyReactGeneratorX)
    RegisterReactGenerator(PropertyType.VECTOR2, Vector2PropertyReactGeneratorX)
    window["IsTypeRegistered"] = true
}