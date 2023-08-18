import {NailManager} from "../IK/NailManager";
import {RigidBody} from "../Components/Physics/RigidBody";
import {huahuoEngine} from "../EngineAPI";
import {b2World} from "@box2d/core";

class PhysicsSystem {
    cppPhysicsSystem

    rigidBodies

    m_world: b2World

    AddRigidBody(rigidBody: RigidBody) {
        let store = huahuoEngine.GetCurrentStore()
        let currentPhysicEnabled = store.IsPhysicsEnabled()
        let physicsManager = store.GetPhysicsManager(true)
        if(!currentPhysicEnabled){
            let gravity = {x: 0.0, y: 100.0}
            physicsManager.SetGravity(gravity['x'], gravity['y'], 0.0)

            // Init the world.
            this.m_world = b2World.Create(gravity)
        }

        let shape = rigidBody.baseShape

        let body = this.m_world.CreateBody()
        body.SetUserData(rigidBody)

        body.SetTransformXY()
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