import {b2BodyType} from "@box2d/core";

class Box2dUtils {
    static typeMatches(stringValue: string, type: b2BodyType): boolean {
        if (stringValue == "static" && type == b2BodyType.b2_staticBody)
            return true
        if (stringValue == "dynamic" && type == b2BodyType.b2_dynamicBody)
            return true
        if (stringValue == "kinematic" && type == b2BodyType.b2_kinematicBody)
            return true

        return false
    }

    static getBodyTypeFromString(stringValue): b2BodyType {
        switch (stringValue) {
            case "static":
                return b2BodyType.b2_staticBody
            case "dynamic":
                return b2BodyType.b2_dynamicBody
            case "kinematic":
                return b2BodyType.b2_kinematicBody
        }

        return null
    }
}

export {Box2dUtils}
