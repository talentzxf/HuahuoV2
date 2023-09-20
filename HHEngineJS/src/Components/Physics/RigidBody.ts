import {AbstractComponent, Component, PropertyValue} from "../AbstractComponent";
import {getPhysicSystem} from "../../PhysicsSystem/PhysicsSystem";
import {b2Body, b2CircleShape, b2Fixture, b2PolygonShape, b2ShapeType, b2Vec2, XY} from "@box2d/core";
import {BaseShapeJS} from "../../Shapes/BaseShapeJS";
import {degToRad, EventParam, GraphEvent, PropertyType, StringProperty} from "hhcommoncomponents";
import {PropertyCategory} from "../PropertySheetBuilder";
import {Box2dUtils} from "./Box2dUtils";
import {GlobalConfig} from "../../GlobalConfig";
import {huahuoEngine} from "../../EngineAPI";
import {ActionParam, GraphAction} from "../../EventGraph/GraphActions";

let physicsToHuahuoScale = GlobalConfig.physicsToHuahuoScale

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

        if (!baseShape.isMirage) {
            getPhysicSystem().AddRigidBody(this)
        }
    }

    onMounted() {
        super.onMounted();

        if (this.baseShape.isMirage)
            return

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

        let _this = this
        shape.registerValueChangeHandler("segments|scaling")(() => {
            if (huahuoEngine.getActivePlayer().isInEditor && !huahuoEngine.getActivePlayer().isPlaying) {
                _this.updateFixture()
            }
        })
    }

    updateFixture() {

        // Change shape type of this fixture.
        let shapeType = Box2dUtils.getShapeTypeFromString(this.colliderShape)

        let currentFixture: b2Fixture = this.body.GetFixtureList()

        let fixtureShape = null
        switch (shapeType) {
            case b2ShapeType.e_polygon:
                fixtureShape = Box2dUtils.getPolygonFromShape(this.baseShape, this.body)
                break;
            case b2ShapeType.e_circle:
                fixtureShape = Box2dUtils.getCircleFromShape(this.baseShape)
                break;
        }

        if (fixtureShape) {
            this.body.CreateFixture({shape: fixtureShape, density: 1})
            this.body.DestroyFixture(currentFixture)
        }
    }

    @GraphEvent()
    OnCollide(@EventParam(PropertyType.COMPONENT) collidedRigidbody, @EventParam(PropertyType.VECTOR2) collisionPoint) {

    }

    colliderWireframeShapePolygon: paper.Path
    colliderWirreframeShapeCircle: paper.Path

    getWorldPoint(localPoint) {
        let worldPoint = {x: -1, y: -1}
        this.body.GetWorldPoint(localPoint, worldPoint)
        return worldPoint
    }

    drawCircleFixture(circleShape: b2CircleShape) {
        if (this.colliderWireframeShapePolygon) {
            this.colliderWireframeShapePolygon.visible = false
        }

        let paperjs = this.baseShape.getPaperJs()

        if (this.colliderWirreframeShapeCircle != null) {
            this.colliderWirreframeShapeCircle.remove()
        }

        let position = this.getWorldPoint(circleShape.m_p)
        this.colliderWirreframeShapeCircle = new paperjs.Path.Circle(
            new paperjs.Point(position.x * GlobalConfig.physicsToHuahuoScale, position.y * GlobalConfig.physicsToHuahuoScale),
            circleShape.m_radius * GlobalConfig.physicsToHuahuoScale)
        this.colliderWirreframeShapeCircle.strokeColor = new paper.Color('blue')
        this.colliderWirreframeShapeCircle.strokeWidth = 2
        this.colliderWirreframeShapeCircle.dashArray = [1, 10, 5, 5];
    }

    drawPolygonFixture(polygonShape: b2PolygonShape) {
        if (this.colliderWirreframeShapeCircle) {
            this.colliderWirreframeShapeCircle.visible = false
        }

        let vertexCount = polygonShape.m_count;
        let vertices = polygonShape.m_vertices;

        let paperjs = this.baseShape.getPaperJs()
        if (this.colliderWireframeShapePolygon == null) {
            this.colliderWireframeShapePolygon = new paperjs.Path({
                strokeColor: 'blue',
                strokeWidth: 2,
                closed: true,
            })

            this.colliderWireframeShapePolygon.dashArray = [1, 10, 5, 5];
        } else { // Keep the shape on top of everything
            let parent = this.colliderWireframeShapePolygon.parent
            parent.addChild(this.colliderWireframeShapePolygon)
        }

        this.colliderWireframeShapePolygon.visible = true

        let currentFrameVertextCount = this.colliderWireframeShapePolygon.segments.length

        if (currentFrameVertextCount > vertexCount) {
            // Remove all unneeded vertices.
            this.colliderWireframeShapePolygon.removeSegment(currentFrameVertextCount - vertexCount)
        }

        for (let vertexIndex = 0; vertexIndex < currentFrameVertextCount; vertexIndex++) {
            let shapePoint = this.getWorldPoint(vertices[vertexIndex])

            this.colliderWireframeShapePolygon.segments[vertexIndex].point.set([
                shapePoint.x * GlobalConfig.physicsToHuahuoScale,
                shapePoint.y * GlobalConfig.physicsToHuahuoScale
            ])
        }

        for (let vertexIndex = currentFrameVertextCount; vertexIndex < vertexCount; vertexIndex++) {
            let shapePoint = this.getWorldPoint(vertices[vertexIndex])

            this.colliderWireframeShapePolygon.add(new paperjs.Point(
                shapePoint.x * GlobalConfig.physicsToHuahuoScale,
                shapePoint.y * GlobalConfig.physicsToHuahuoScale
            ))
        }
    }

    @GraphAction(true)
    setVelocity(@ActionParam(PropertyType.VECTOR2) velocity: XY) {
        this.getBody().SetLinearVelocity({
            x: velocity.x,
            y: velocity.y
        })
    }

    @GraphAction(true)
    reset() {
        let shape = this.baseShape
        this.getBody().SetLinearVelocity(b2Vec2.ZERO)
        this.getBody().SetAngularVelocity(0)
        this.getBody().SetTransformVec({
            x: shape.getPosition(false).x / physicsToHuahuoScale,
            y: shape.getPosition(false).y / physicsToHuahuoScale,
        }, degToRad(shape.getRotation(false)))
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (this.isMirage) // Do nothing for migrate shapes.
            return

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

                if (!Box2dUtils.shapeTypeMatches(this.colliderShape, fixture.GetShape().GetType())) {
                    this.updateFixture()
                    fixture = this.body.GetFixtureList()
                }

                switch (fixture.GetType()) {
                    case b2ShapeType.e_polygon:
                        this.drawPolygonFixture(fixture.GetShape() as b2PolygonShape)
                        break;
                    case b2ShapeType.e_circle:
                        this.drawCircleFixture(fixture.GetShape() as b2CircleShape)
                        break;
                }
            }
        }
    }
}

export {RigidBody}