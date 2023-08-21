import {RigidBody} from "../Components/Physics/RigidBody";
import {huahuoEngine} from "../EngineAPI";
import {b2BodyType, b2Gjk, b2MakeArray, b2PolygonShape, b2Toi, b2Vec2, b2World} from "@box2d/core";

let physicsToHuahuoScale = 40.0

function getPolygonFromShape(shape, body) {
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

function degToRad(degree: number) {
    return degree / 180 * Math.PI
}

function radToDeg(rad: number) {
    return rad / Math.PI * 180
}

class PhysicsSystem {
    cppPhysicsSystem

    rigidBodies

    m_world: b2World

    AddRigidBody(rigidBody: RigidBody) {
        let store = huahuoEngine.GetCurrentStore()
        let currentPhysicEnabled = store.IsPhysicsEnabled()
        let physicsManager = store.GetPhysicsManager(true)
        if (!currentPhysicEnabled) {
            let gravity = {x: 0.0, y: 10.0}
            physicsManager.SetGravity(gravity['x'], gravity['y'], 0.0)

            // Init the world.
            this.m_world = b2World.Create(gravity)
        }

        let shape = rigidBody.baseShape
        let type = b2BodyType.b2_dynamicBody
        if (rigidBody.isStatic) {
            type = b2BodyType.b2_staticBody
        }

        let body = this.m_world.CreateBody({
            type: type,
            position: {
                x: shape.position.x / physicsToHuahuoScale,
                y: shape.position.y / physicsToHuahuoScale
            },
            angle: degToRad(shape.rotation)
        })

        let polygonShape = getPolygonFromShape(shape, body)


        body.CreateFixture({
            shape: polygonShape,
            density: 1,
        })

        body.SetUserData(rigidBody)
        rigidBody.setBody(body)
    }

    Reset() {
        b2Gjk.reset()
        b2Toi.reset()
    }

    lastUpdateTime = 0

    Step(elapsedTime) {
        if (huahuoEngine.GetCurrentStore().IsPhysicsEnabled()) {
            this.m_world.Step(elapsedTime, {
                velocityIterations: 8,
                positionIterations: 3
            })

            for (let b = this.m_world.GetBodyList(); b; b = b.GetNext()) {
                let rigidBody = b.GetUserData()
                if (rigidBody) {
                    let shape = rigidBody.baseShape
                    const xf = b.GetTransform();

                    let globalPosition = new paper.Point(xf.GetPosition().x * physicsToHuahuoScale, xf.GetPosition().y * physicsToHuahuoScale)
                    shape.getActor().setPosition(globalPosition.x, globalPosition.y)
                    shape.getActor().setRotation(radToDeg(xf.GetAngle()))
                }
            }
        }
    }
}


function getPhysicSystem(): PhysicsSystem {
    let physicsSystem = window["PhysicsSystem"]
    if (!physicsSystem) {
        physicsSystem = new PhysicsSystem()
        window["PhysicsSystem"] = physicsSystem
    }

    return physicsSystem
}

export {getPhysicSystem}