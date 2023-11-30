import {RegisterDivGenerator} from "../Inspector/BasePropertyDivGenerator";
import {PropertyType} from "hhcommoncomponents"
import {panelPropertyDivGenerator} from "../Inspector/PanelPropertyDivGenerator";
import {booleanPropertyDivGenerator} from "../Inspector/BooleanPropertyDivGenerator";
import {keyFramesPropertyDivGenerator} from "../Inspector/KeyFramesPropertyDivGenerator";
import {subComponentsDivGenerator} from "../Inspector/SubComponentsDivGenerator";
import {vector3PropertyGenerator} from "../Inspector/Vector3PropertyDivGenerator";
import {PanelPropertyX,} from "./PanelPropertyX";
import {StringPropertyX} from "./StringPropertyX";
import {Vector2PropertyX} from "./Vector2PropertyX";
import {FloatPropertyX} from "./FloatPropertyX";
import {RegisterReactGenerator} from "./BasePropertyX";
import {ColorPropertyX} from "./ColorPropertyX";
import {ComponentPropertyX} from "./ComponentPropertyX";
import {ArrayPropertyX} from "./ArrayPropertyX";
import {ShapePropertyX} from "./ShapePropertyX";
import {CustomFieldPropertyX} from "./CustomFieldPropertyX";
import {ButtonPropertyX} from "./ButtonPropertyX";
import {ColorStopArrayPropertyX} from "./ColorStopArrayPropertyX";
import {GroupPropertyX} from "./GroupPropertyX";
import {KeyFramePropertyX} from "./KeyFramePropertyX";
import {SegmentPropertyX} from "./SegmentPropertyX";

// Avoid being imported twice.
// TODO: Any less dirty approach ??
if (!window["IsTypesRegistered"]) {
    RegisterDivGenerator(PropertyType.BOOLEAN, booleanPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.VECTOR3, vector3PropertyGenerator)
    RegisterDivGenerator(PropertyType.PANEL, panelPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.KEYFRAMES, keyFramesPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.SUBCOMPONENTARRAY, subComponentsDivGenerator)

    RegisterReactGenerator(PropertyType.KEYFRAMES, KeyFramePropertyX)
    RegisterReactGenerator(PropertyType.GROUP, GroupPropertyX)
    RegisterReactGenerator(PropertyType.COLOR, ColorPropertyX)
    RegisterReactGenerator(PropertyType.BUTTON, ButtonPropertyX)
    RegisterReactGenerator(PropertyType.COMPONENT, ComponentPropertyX)
    RegisterReactGenerator(PropertyType.SEGMENT, SegmentPropertyX)
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