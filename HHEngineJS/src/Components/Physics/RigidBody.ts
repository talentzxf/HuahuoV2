import {AbstractComponent, Component} from "../AbstractComponent";
import {getPhysicSystem} from "../../PhysicsSystem/PhysicsSystem";
import {b2Body} from "@box2d/core";
import {BaseShapeJS} from "../../Shapes/BaseShapeJS";

@Component({compatibleShapes: ["ElementShapeJS"], maxCount: 1})
class RigidBody extends AbstractComponent{
    private body: b2Body

    isStatic: boolean = false

    setBody(body: b2Body) {
        this.body = body
    }

    getBody(){
        return this.body
    }

    override setBaseShape(baseShape: BaseShapeJS) {
        super.setBaseShape(baseShape)

        getPhysicSystem().AddRigidBody(this)
    }
}

export {RigidBody}