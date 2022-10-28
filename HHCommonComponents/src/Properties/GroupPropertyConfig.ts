import {PropertyConfig} from "./PropertyConfig";
import {Property} from "./PropertySheet";

class GroupPropertyConfig extends PropertyConfig{
    children: Array<Property> // For Group property
}

export {GroupPropertyConfig}