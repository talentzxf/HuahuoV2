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
        let rectangleShape: RectangleJS = new RectangleJS();
        rectangleShape.addComponent(new StrokeComponent())
        rectangleShape.addComponent(new FillColorComponent())
        rectangleShape.addComponent(new RigidBody())

        let fillComponent = rectangleShape.getComponentByTypeName("FillColorComponent")
        fillComponent["fillColor"] = {red: 1.0, green: 0, blue: 1.0, alpha: 1.0}

        rectangleShape.setStartPoint(new Vector2(0, 0))
        rectangleShape.setEndPoint(new Vector2(100, 100))
        rectangleShape.position = new Vector2(100, 100)

        huahuoEngine.GetCurrentLayer().addShape(rectangleShape)
        // animationPlayer.setFrameId(60)
        //
        // fillComponent["fillColor"] = {red: 0, green: 0, blue: 1.0, alpha: 1.0}
        //
        // rectangleShape.position = new paper.Point(500.0, 500.0)

        animationPlayer.startPlay()
    })
}

export {initTest}