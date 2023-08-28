import {RigidBody} from "../Components/Physics/RigidBody";
import {huahuoEngine} from "../EngineAPI";
import {
    b2Contact,
    b2ContactListener,
    b2GetPointStates,
    b2Gjk,
    b2Manifold,
    b2PointState,
    b2Toi,
    b2Vec2,
    b2World,
    b2WorldManifold
} from "@box2d/core";
import {GlobalConfig} from "../GlobalConfig";
import {ContactPoint, k_maxContactPoints} from "./ContactPoint";
import {Box2dUtils} from "../Components/Physics/Box2dUtils";
import {radToDeg, degToRad} from "hhcommoncomponents";

let physicsToHuahuoScale = GlobalConfig.physicsToHuahuoScale

class PhysicsSystem extends b2ContactListener {

    public readonly m_points = Array.from({length: k_maxContactPoints}, () => new ContactPoint());

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

            this.m_world.SetContactListener(this)
        }

        let shape = rigidBody.baseShape
        let type = Box2dUtils.getBodyTypeFromString(rigidBody.rigidBodyType)

        let body = this.m_world.CreateBody({
            type: type,
            position: {
                x: shape.position.x / physicsToHuahuoScale,
                y: shape.position.y / physicsToHuahuoScale
            },
            angle: degToRad(shape.rotation)
        })

        let polygonShape = Box2dUtils.getPolygonFromShape(shape, body)

        body.CreateFixture({
            shape: polygonShape,
            density: 1,
        })

        body.SetUserData(rigidBody)
        rigidBody.setBody(body)
    }

    Reset() {
        // Reset all rigidbodies
        for (let b = this.m_world.GetBodyList(); b; b = b.GetNext()) {
            let rigidBody = b.GetUserData()
            let shape = null
            if (rigidBody) {
                shape = rigidBody.baseShape
                b.SetTransformVec({
                    x: shape.position.x / physicsToHuahuoScale,
                    y: shape.position.y / physicsToHuahuoScale
                }, degToRad(shape.rotation))
            }

            b.SetLinearVelocity(b2Vec2.ZERO);
            b.SetAngularVelocity(0.0)

            if (shape) {
                shape.update()
            }
        }

        b2Gjk.reset()
        b2Toi.reset()
    }

    lastUpdateTime = 0

    PreSolve(contact: b2Contact, oldManifold: b2Manifold) {
        super.PreSolve(contact, oldManifold);

        let manifold = contact.GetManifold()
        if (manifold.pointCount == 0)
            return

        let state1: b2PointState[] = []
        let state2: b2PointState[] = []
        b2GetPointStates(state1, state2, oldManifold, manifold);

        let worldManifold = new b2WorldManifold();
        contact.GetWorldManifold(worldManifold)

        let body1 = contact.GetFixtureA().GetBody()
        let body2 = contact.GetFixtureB().GetBody()

        let rigidBody1 = body1.GetUserData() as RigidBody
        let rigidBody2 = body2.GetUserData() as RigidBody

        if (rigidBody1) {
            rigidBody1.OnCollide(rigidBody2, worldManifold.points[0])
        }

        if (rigidBody2) {
            rigidBody2.OnCollide(rigidBody1, worldManifold.points[0])
        }
    }

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