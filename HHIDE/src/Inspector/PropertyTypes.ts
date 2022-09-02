import {RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {vector2PropertyGenerator} from "./Vector2PropertyDivGenerator";
import {PropertyType} from "hhcommoncomponents"
import {floatPropertyDivGenerator} from "./FloatPropertyDivGenerator";
import {colorPropertyDivGenerator} from "./ColorPropertyDivGenerator";
import {stringPropertyDivGenerator} from "./StringPropertyDivGenerator";
import {buttonPropertyDivGenerator} from "./ButtonPropertyDivGenerator";
import {groupPropertyDivGenerator} from "./GroupPropertyDivGenerator";


// Avoid being imported twice.
// TODO: Any less dirty approach ??
if(!window["IsTypesRegistered"]){
    RegisterDivGenerator(PropertyType.COLOR, colorPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.FLOAT, floatPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.VECTOR2, vector2PropertyGenerator)
    RegisterDivGenerator(PropertyType.STRING, stringPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.BUTTON, buttonPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.GROUP, groupPropertyDivGenerator)
    window["IsTypeRegistered"] = true
}