import {RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {PropertyType} from "hhcommoncomponents"
import {groupPropertyDivGenerator} from "./GroupPropertyDivGenerator";
import {panelPropertyDivGenerator} from "./PanelPropertyDivGenerator";
import {booleanPropertyDivGenerator} from "./BooleanPropertyDivGenerator";
import {keyFramesPropertyDivGenerator} from "./KeyFramesPropertyDivGenerator";
import {subComponentsDivGenerator} from "./SubComponentsDivGenerator";
import {vector3PropertyGenerator} from "./Vector3PropertyDivGenerator";
import {PanelPropertyX,} from "../InspectorX/PanelPropertyX";
import {StringPropertyX} from "../InspectorX/StringPropertyX";
import {Vector2PropertyX} from "../InspectorX/Vector2PropertyX";
import {FloatPropertyX} from "../InspectorX/FloatPropertyX";
import {RegisterReactGenerator} from "../InspectorX/BasePropertyX";
import {ColorPropertyX} from "../InspectorX/ColorPropertyX";
import {ComponentPropertyX} from "../InspectorX/ComponentPropertyX";
import {ArrayPropertyX} from "../InspectorX/ArrayPropertyX";
import {ShapePropertyX} from "../InspectorX/ShapePropertyX";
import {CustomFieldPropertyX} from "../InspectorX/CustomFieldPropertyX";
import {ButtonPropertyX} from "../InspectorX/ButtonPropertyX";
import {ColorStopArrayPropertyX} from "../InspectorX/ColorStopArrayPropertyX";

// Avoid being imported twice.
// TODO: Any less dirty approach ??
if (!window["IsTypesRegistered"]) {
    RegisterDivGenerator(PropertyType.BOOLEAN, booleanPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.VECTOR3, vector3PropertyGenerator)
    RegisterDivGenerator(PropertyType.GROUP, groupPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.PANEL, panelPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.KEYFRAMES, keyFramesPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.SUBCOMPONENTARRAY, subComponentsDivGenerator)

    RegisterReactGenerator(PropertyType.COLOR, ColorPropertyX)
    RegisterReactGenerator(PropertyType.BUTTON, ButtonPropertyX)
    RegisterReactGenerator(PropertyType.COMPONENT, ComponentPropertyX)
    RegisterReactGenerator(PropertyType.PANEL, PanelPropertyX)
    RegisterReactGenerator(PropertyType.STRING, StringPropertyX)
    RegisterReactGenerator(PropertyType.NUMBER, FloatPropertyX)
    RegisterReactGenerator(PropertyType.VECTOR2, Vector2PropertyX)
    RegisterReactGenerator(PropertyType.ARRAY, ArrayPropertyX)
    RegisterReactGenerator(PropertyType.SHAPE, ShapePropertyX)
    RegisterReactGenerator(PropertyType.CUSTOMFIELD, CustomFieldPropertyX)
    RegisterReactGenerator(PropertyType.COLORSTOPARRAY, ColorStopArrayPropertyX)
    window["IsTypeRegistered"] = true
}