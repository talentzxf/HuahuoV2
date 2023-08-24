import {AbstractComponent, Component, PropertyValue} from "../AbstractComponent";
import {getPhysicSystem} from "../../PhysicsSystem/PhysicsSystem";
import {b2Body} from "@box2d/core";
import {BaseShapeJS} from "../../Shapes/BaseShapeJS";
import {EventParam, GraphEvent, PropertyType} from "hhcommoncomponents";
import {PropertyCategory} from "../PropertySheetBuilder";
import {StringProperty} from "hhcommoncomponents";
import {Box2dUtils} from "./Box2dUtils";

@Component({compatibleShapes: ["BaseSolidShape"], maxCount: 1})
class RigidBody extends AbstractComponent {
    private body: b2Body

    @PropertyValue(PropertyCategory.stringValue, "dynamic", {options:["dynamic", "kinematic", "static"]} as StringProperty)
    rigidBodyType

    setBody(body: b2Body) {
        this.body = body
    }

    getBody() {
        return this.body
    }

    override setBaseShape(baseShape: BaseShapeJS) {
        super.setBaseShape(baseShape)

        getPhysicSystem().AddRigidBody(this)
    }

    @GraphEvent()
    OnCollide(@EventParam(PropertyType.COMPONENT) collidedRigidbody, @EventParam(PropertyType.VECTOR2) collisionPoint) {

    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if(this.body){
            if(!Box2dUtils.typeMatches(this.rigidBodyType, this.body.GetType())){
                let currentRigidBodyType = Box2dUtils.getBodyTypeFromString(this.rigidBodyType)

                this.body.SetType(currentRigidBodyType)
            }
        }
    }
}

export {RigidBody}