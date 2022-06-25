import {RegisterDivGenerator} from "./BasePropertyDivGenerator";
import {vector2PropertyGenerator} from "./Vector2PropertyDivGenerator";
import {PropertyType} from "hhcommoncomponents"


// Avoid being imported twice.
// TODO: Any less dirty approach ??
if(!window["IsTypesRegistered"]){
    RegisterDivGenerator(PropertyType.VECTOR2, vector2PropertyGenerator)
    window["IsTypeRegistered"] = true
}