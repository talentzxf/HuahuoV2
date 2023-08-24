import {AbstractComponent, Component, PropertyValue} from "../AbstractComponent";
import {getPhysicSystem} from "../../PhysicsSystem/PhysicsSystem";
import {b2Body} from "@box2d/core";
import {BaseShapeJS} from "../../Shapes/BaseShapeJS";
import {EventParam, GraphEvent, PropertyType} from "hhcommoncomponents";
import {PropertyCategory} from "../PropertySheetBuilder";
import {StringProperty} from "hhcommoncomponents";
import {Box2dUtils} from "./Box2dUtils";
import {radToDeg, degToRad} from "hhcommoncomponents";
import {GlobalConfig} from "../../GlobalConfig";
import {huahuoEngine} from "../../EngineAPI";

@Component({compatibleShapes: ["BaseSolidShape"], maxCount: 1})
class RigidBody extends AbstractComponent {
    private body: b2Body

    @PropertyValue(PropertyCategory.stringValue, "dynamic", {options: ["dynamic", "kinematic", "static"]} as StringProperty)
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

    onMounted() {
        super.onMounted();

        let shape = this.baseShape
        let body = this.body
        shape.registerValueChangeHandler("position|rotation")(() => { // BaseShape's position moved. Align the rigid body's position with baseShape.
            if (huahuoEngine.getActivePlayer().isInEditor && !huahuoEngine.getActivePlayer().isPlaying) {
                body.SetTransformVec({
                    x: shape.position.x / GlobalConfig.physicsToHuahuoScale,
                    y: shape.position.y / GlobalConfig.physicsToHuahuoScale
                }, degToRad(shape.rotation))
            }
        })
    }

    @GraphEvent()
    OnCollide(@EventParam(PropertyType.COMPONENT) collidedRigidbody, @EventParam(PropertyType.VECTOR2) collisionPoint) {

    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (this.body) {
            if (!Box2dUtils.typeMatches(this.rigidBodyType, this.body.GetType())) {
                let currentRigidBodyType = Box2dUtils.getBodyTypeFromString(this.rigidBodyType)

                this.body.SetType(currentRigidBodyType)
            }
        }
    }
}

export {RigidBody}