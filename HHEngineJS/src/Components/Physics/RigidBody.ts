import {AbstractComponent, Component, PropertyValue} from "../AbstractComponent";
import {getPhysicSystem} from "../../PhysicsSystem/PhysicsSystem";
import {b2Body, b2Fixture, b2PolygonShape, b2ShapeType} from "@box2d/core";
import {BaseShapeJS} from "../../Shapes/BaseShapeJS";
import {degToRad, EventParam, GraphEvent, PropertyType, StringProperty} from "hhcommoncomponents";
import {PropertyCategory} from "../PropertySheetBuilder";
import {Box2dUtils} from "./Box2dUtils";
import {GlobalConfig} from "../../GlobalConfig";
import {huahuoEngine} from "../../EngineAPI";

@Component({compatibleShapes: ["BaseSolidShape"], maxCount: 1})
class RigidBody extends AbstractComponent {
    private body: b2Body

    @PropertyValue(PropertyCategory.stringValue, "dynamic", {options: ["dynamic", "kinematic", "static"]} as StringProperty)
    rigidBodyType

    @PropertyValue(PropertyCategory.stringValue, "polygon", {options: ["polygon", "circle", "edge"]})
    colliderShape

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

        shape.registerValueChangeHandler("segments|scaling")(() => {
            if (huahuoEngine.getActivePlayer().isInEditor && !huahuoEngine.getActivePlayer().isPlaying) {
                let currentFixture: b2Fixture = body.GetFixtureList()
                let polygonShape = Box2dUtils.getPolygonFromShape(this.baseShape, this.body)
                body.CreateFixture({shape: polygonShape, density: 1})
                body.DestroyFixture(currentFixture)
            }
        })
    }

    @GraphEvent()
    OnCollide(@EventParam(PropertyType.COMPONENT) collidedRigidbody, @EventParam(PropertyType.VECTOR2) collisionPoint) {

    }

    colliderWireframeShape: paper.Path

    drawPolygonFixture(polygonShape: b2PolygonShape) {
        let _body = this.body
        function getWorldPoint(localPoint){
            let worldPoint = {x: -1, y: -1}
            _body.GetWorldPoint(localPoint, worldPoint)
            return worldPoint
        }

        let vertexCount = polygonShape.m_count;
        let vertices = polygonShape.m_vertices;

        let paperjs = this.baseShape.getPaperJs()
        if (this.colliderWireframeShape == null) {
            this.colliderWireframeShape = new paperjs.Path({
                strokeColor: 'black',
                closed: true
            })
        }

        let currentFrameVertextCount = this.colliderWireframeShape.segments.length

        if (currentFrameVertextCount > vertexCount) {
            // Remove all unneeded vertices.
            this.colliderWireframeShape.removeSegment(currentFrameVertextCount - vertexCount)
        }

        for (let vertexIndex = 0; vertexIndex < currentFrameVertextCount; vertexIndex++) {
            let shapePoint = getWorldPoint(vertices[vertexIndex])

            this.colliderWireframeShape.segments[vertexIndex].point.set([
                shapePoint.x * GlobalConfig.physicsToHuahuoScale,
                shapePoint.y * GlobalConfig.physicsToHuahuoScale
            ])
        }

        for (let vertexIndex = currentFrameVertextCount; vertexIndex < vertexCount; vertexIndex++) {
            let shapePoint = getWorldPoint(vertices[vertexIndex])

            this.colliderWireframeShape.add(new paperjs.Point(
                shapePoint.x * GlobalConfig.physicsToHuahuoScale,
                shapePoint.y * GlobalConfig.physicsToHuahuoScale
            ))
        }
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (this.body) {
            if (!Box2dUtils.typeMatches(this.rigidBodyType, this.body.GetType())) {
                let currentRigidBodyType = Box2dUtils.getBodyTypeFromString(this.rigidBodyType)

                this.body.SetType(currentRigidBodyType)
            }

            this.body.SetAwake(this.isComponentActive())
            console.log("Body IsAwake:" + this.body.IsAwake())

            // Draw collider wireframe in Editor.
            if (huahuoEngine.getActivePlayer().isInEditor) {
                let fixture = this.body.GetFixtureList()
                switch (fixture.GetType()) {
                    case b2ShapeType.e_polygon:
                        this.drawPolygonFixture(fixture.GetShape() as b2PolygonShape)
                        break;
                }
            }
        }
    }
}

export {RigidBody}