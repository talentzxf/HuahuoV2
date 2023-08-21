import {huahuoEngine, Player, RectangleJS, renderEngine2D, RigidBody} from "../src";
import {Vector2} from "hhcommoncomponents";
import {StrokeComponent} from "../src/Components/StrokeComponent";
import {FillColorComponent} from "../src/Components/FillColorComponent";

import {
    b2Body,
    b2BodyType,
    b2EdgeShape,
    b2Gjk,
    b2PolygonShape,
    b2RandomFloat,
    b2Toi,
    b2Vec2,
    b2World,
    b2Fixture
} from "@box2d/core"

let animationPlayer = null

function prepareEnvironment() {
    console.log("Engine inited!")

    let canvas = document.querySelector("#huahuoCanvas") as HTMLCanvasElement
    renderEngine2D.init(canvas, true)
    animationPlayer = new Player(huahuoEngine.GetCurrentStoreId())
    huahuoEngine.setActivePlayer(animationPlayer)

    let layer = huahuoEngine.GetCurrentStore().CreateLayer("FirstLayer")
    layer.GetTimeLineCellManager().MergeCells(0, huahuoEngine.defaultFrameCount)
}

function initTest() {
    huahuoEngine.ExecuteAfterInited(() => {
        prepareEnvironment()

        animationPlayer.setFrameId(0)
        {
            let rectangleShape: RectangleJS = new RectangleJS();
            rectangleShape.addComponent(new StrokeComponent())
            rectangleShape.addComponent(new FillColorComponent())

            rectangleShape.setStartPoint(new Vector2(0, 0))
            rectangleShape.setEndPoint(new Vector2(100, 100))
            rectangleShape.position = new Vector2(100, 100)
            rectangleShape.rotation = 46

            rectangleShape.addComponent(new RigidBody())

            let fillComponent = rectangleShape.getComponentByTypeName("FillColorComponent")
            fillComponent["fillColor"] = {red: 1.0, green: 0, blue: 1.0, alpha: 1.0}

            huahuoEngine.GetCurrentLayer().addShape(rectangleShape)
        }

        {
            let bullet: RectangleJS = new RectangleJS()
            bullet.addComponent(new StrokeComponent())
            bullet.addComponent(new FillColorComponent())
            bullet.setStartPoint(new Vector2(300, 10))
            bullet.setEndPoint(new Vector2(310, 20))

            let rigidBody = new RigidBody()
            bullet.addComponent(rigidBody)
            rigidBody.getBody().SetBullet(true)
            rigidBody.getBody().SetLinearVelocity({
                x: -50.0,
                y: 0.0
            })

            huahuoEngine.GetCurrentLayer().addShape(bullet)
        }


        {
            let floor: RectangleJS = new RectangleJS()
            floor.addComponent(new StrokeComponent())
            floor.addComponent(new FillColorComponent())

            floor.setStartPoint(new Vector2(0, 480))
            floor.setEndPoint(new Vector2(500, 500))
            floor.position = new Vector2(250, 500)
            let rigidBody = new RigidBody()
            rigidBody.isStatic = true
            floor.addComponent(rigidBody)
            huahuoEngine.GetCurrentLayer().addShape(floor)
        }
        animationPlayer.startPlay()
    })
}

export {initTest}