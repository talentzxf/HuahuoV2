import {RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {vector2PropertyGenerator} from "./Vector2PropertyDivGenerator";
import {PropertyType} from "hhcommoncomponents"
import {floatPropertyDivGenerator} from "./FloatPropertyDivGenerator";
import {colorPropertyDivGenerator} from "./ColorPropertyDivGenerator";


// Avoid being imported twice.
// TODO: Any less dirty approach ??
if(!window["IsTypesRegistered"]){
    RegisterDivGenerator(PropertyType.COLOR, colorPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.FLOAT, floatPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.VECTOR2, vector2PropertyGenerator)
    window["IsTypeRegistered"] = true
}