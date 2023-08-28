import {b2BodyType, b2CircleShape, b2MakeArray, b2PolygonShape, b2ShapeType, b2Vec2} from "@box2d/core";
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

    static getShapeTypeFromString(stringValue): b2ShapeType {
        switch (stringValue) {
            case 'polygon':
                return b2ShapeType.e_polygon
            case 'edge':
                return b2ShapeType.e_edge
            case 'circle':
                return b2ShapeType.e_circle
        }
        return null
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

    static shapeTypeMatches(shapeTypeString: string, shapeType: b2ShapeType): boolean {
        if (shapeTypeString == "polygon" && shapeType == b2ShapeType.e_polygon) {
            return true
        } else if (shapeTypeString == "circle" && shapeType == b2ShapeType.e_circle) {
            return true
        } else if (shapeTypeString == "edge" && shapeType == b2ShapeType.e_edge) {
            return true
        }

        return false
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

    static getCircleFromShape(shape, body){
        let shapeGlobalPosition = shape.localToGlobal(shape.position).multiply(1/physicsToHuahuoScale)
        let shapeArea = shape.paperShape.area
        let radius = Math.sqrt(shapeArea/Math.PI)

        let circleShape = new b2CircleShape()
        circleShape.Set(shapeGlobalPosition, radius/physicsToHuahuoScale)
        return circleShape
    }
}

export {Box2dUtils}
