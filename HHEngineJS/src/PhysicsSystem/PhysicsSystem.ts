import {RigidBody} from "../Components/Physics/RigidBody";
import {huahuoEngine} from "../EngineAPI";
import {
    b2BodyType,
    b2Contact,
    b2ContactListener,
    b2GetPointStates,
    b2Gjk,
    b2MakeArray,
    b2Manifold,
    b2PointState,
    b2PolygonShape,
    b2Toi,
    b2Vec2,
    b2World,
    b2WorldManifold
} from "@box2d/core";
import {GlobalConfig} from "../GlobalConfig";
import {ContactPoint, k_maxContactPoints} from "./ContactPoint";

let physicsToHuahuoScale = GlobalConfig.physicsToHuahuoScale

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



class PhysicsSystem extends b2ContactListener{

    public readonly m_points = Array.from({ length: k_maxContactPoints }, () => new ContactPoint());

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
        let type = b2BodyType.b2_dynamicBody
        if (rigidBody.rigidBodyType == "kinematic") {
            type = b2BodyType.b2_kinematicBody
        } else if (rigidBody.rigidBodyType == "static"){
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
        // Reset all rigidbodies
        for (let b = this.m_world.GetBodyList(); b; b = b.GetNext()) {
            let rigidBody = b.GetUserData()
            if (rigidBody) {
                let shape = rigidBody.baseShape
                b.SetTransformVec({
                    x: shape.position.x/physicsToHuahuoScale,
                    y: shape.position.y/physicsToHuahuoScale
                }, 0)
            }

            b.SetLinearVelocity(b2Vec2.ZERO);
            b.SetAngularVelocity(0.0)
        }

        b2Gjk.reset()
        b2Toi.reset()
    }

    lastUpdateTime = 0

    PreSolve(contact: b2Contact, oldManifold: b2Manifold) {
        super.PreSolve(contact, oldManifold);

        let manifold = contact.GetManifold()
        if(manifold.pointCount == 0)
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

        if(rigidBody1){
            rigidBody1.OnCollide(rigidBody2, worldManifold.points[0])
        }

        if(rigidBody2){
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