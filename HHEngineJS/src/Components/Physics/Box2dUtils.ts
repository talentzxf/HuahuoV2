import {b2BodyType, b2MakeArray, b2PolygonShape, b2Vec2} from "@box2d/core";
import {GlobalConfig} from "../../GlobalConfig";
let physicsToHuahuoScale = GlobalConfig.physicsToHuahuoScale

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

    static getPolygonFromShape(shape, body) {
        let segments = shape.getSegments()
        let vertices = b2MakeArray(segments.length, b2Vec2)
        for (let i = 0; i < segments.length; i++) {
            let globalPoint = shape.localToGlobal(segments[i].point).multiply(1 / physicsToHuahuoScale)
            let bodyLocalPoint = {
                x: 0.0,
                y: 0.0
            }
            body.GetLocalPoint({
                x: globalPoint.x,
                y: globalPoint.y
            }, bodyLocalPoint)
            vertices[i].Set(bodyLocalPoint.x, bodyLocalPoint.y)
        }

        let polygonShape = new b2PolygonShape()
        polygonShape.Set(vertices, segments.length)

        return polygonShape
    }
}

export {Box2dUtils}
