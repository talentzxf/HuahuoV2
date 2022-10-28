import {RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {vector2PropertyGenerator} from "./Vector2PropertyDivGenerator";
import {PropertyType} from "hhcommoncomponents"
import {floatPropertyDivGenerator} from "./FloatPropertyDivGenerator";
import {colorPropertyDivGenerator} from "./ColorPropertyDivGenerator";
import {stringPropertyDivGenerator} from "./StringPropertyDivGenerator";
import {buttonPropertyDivGenerator} from "./ButtonPropertyDivGenerator";
import {groupPropertyDivGenerator} from "./GroupPropertyDivGenerator";
import {referencePropertyDivGenerator} from "./ReferencePropertyDivGenerator";
import {panelPropertyDivGenerator} from "./PanelPropertyDivGenerator";
import {arrayPropertyDivGenerator} from "./ArrayPropertyDivGenerator";
import {Property} from "properties-file";


// Avoid being imported twice.
// TODO: Any less dirty approach ??
if(!window["IsTypesRegistered"]){
    RegisterDivGenerator(PropertyType.COLOR, colorPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.FLOAT, floatPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.VECTOR2, vector2PropertyGenerator)
    RegisterDivGenerator(PropertyType.STRING, stringPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.BUTTON, buttonPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.GROUP, groupPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.REFERENCE, referencePropertyDivGenerator)
    RegisterDivGenerator(PropertyType.PANEL, panelPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.ARRAY, arrayPropertyDivGenerator)
    window["IsTypeRegistered"] = true
}