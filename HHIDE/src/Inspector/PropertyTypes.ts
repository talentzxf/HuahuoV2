import {RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {vector2PropertyGenerator} from "./Vector2PropertyDivGenerator";
import {PropertyType} from "hhcommoncomponents"
import {floatPropertyDivGenerator} from "./FloatPropertyDivGenerator";


// Avoid being imported twice.
// TODO: Any less dirty approach ??
if(!window["IsTypesRegistered"]){
    RegisterDivGenerator(PropertyType.FLOAT, floatPropertyDivGenerator)
    RegisterDivGenerator(PropertyType.VECTOR2, vector2PropertyGenerator)
    window["IsTypeRegistered"] = true
}