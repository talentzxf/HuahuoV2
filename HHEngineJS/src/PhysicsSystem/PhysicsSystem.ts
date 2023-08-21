import {RigidBody} from "../Components/Physics/RigidBody";
import {huahuoEngine} from "../EngineAPI";
import {b2BodyType, b2Gjk, b2PolygonShape, b2Toi, b2World} from "@box2d/core";
import {GlobalConfig} from "../GlobalConfig";

function degToRad(degree: number) {
    return degree / 180 * Math.PI
}

function radToDeg(rad: number){
    return rad/Math.PI * 180
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
            let gravity = {x: 0.0, y: 1000.0}
            physicsManager.SetGravity(gravity['x'], gravity['y'], 0.0)

            // Init the world.
            this.m_world = b2World.Create(gravity)
        }

        let shape = rigidBody.baseShape

        let boundBox = shape.getBounds()

        // TODO: Create collider component.
        const box = new b2PolygonShape()
        box.SetAsBox(boundBox.width / 2.0, boundBox.height / 2.0, {
            x: 0.0,
            y: 0.0,
        }, 0.0)

        let type = b2BodyType.b2_dynamicBody
        if (rigidBody.isStatic) {
            type = b2BodyType.b2_staticBody
        }

        let body = this.m_world.CreateBody({
            type: type,
        })
        body.CreateFixture({
            shape: box,
            density: 1,
        })

        body.SetTransformVec({
            x: shape.position.x,
            y: shape.position.y
        }, degToRad(shape.rotation))

        body.SetUserData(rigidBody)
        rigidBody.setBody(body)
    }

    Reset() {
        b2Gjk.reset()
        b2Toi.reset()
    }

    lastUpdateTime = 0,

    Step() {
        if (huahuoEngine.GetCurrentStore().IsPhysicsEnabled()) {
            this.m_world.Step(1 / GlobalConfig.fps, {
                velocityIterations: 8,
                positionIterations: 3
            })

            for (let b = this.m_world.GetBodyList(); b; b = b.GetNext()) {
                let rigidBody = b.GetUserData()
                if (rigidBody) {
                    let shape = rigidBody.baseShape
                    const xf = b.GetTransform();

                    shape.getActor().setPosition(xf.GetPosition().x, xf.GetPosition().y)
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