import {AbstractComponent} from "./AbstractComponent";
import {b2Body} from "@box2d/core";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {getPhysicSystem} from "../PhysicsSystem/PhysicsSystem";

class RigidBody extends AbstractComponent {
    private body: b2Body

    override setBaseShape(baseShape: BaseShapeJS) {
        super.setBaseShape(baseShape)

        getPhysicSystem().AddRigidBody(this)
    }
}