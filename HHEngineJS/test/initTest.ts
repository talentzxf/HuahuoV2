import {huahuoEngine, Player, RectangleJS, renderEngine2D} from "../src";
import {Vector2} from "hhcommoncomponents";
import {StrokeComponent} from "../src/Components/StrokeComponent";
import {FillColorComponent} from "../src/Components/FillColorComponent";

let animationPlayer = null

function initTest() {
    huahuoEngine.ExecuteAfterInited(() => {
        console.log("Engine inited!")

        let canvas = document.querySelector("#huahuoCanvas") as HTMLCanvasElement
        renderEngine2D.init(canvas, true)
        animationPlayer = new Player(huahuoEngine.GetCurrentStoreId())
        huahuoEngine.setActivePlayer(animationPlayer)

        let layer = huahuoEngine.GetCurrentStore().CreateLayer("FirstLayer")
        layer.GetTimeLineCellManager().MergeCells(0, huahuoEngine.defaultFrameCount)

        let rectangleShape: RectangleJS = new RectangleJS();
        rectangleShape.addComponent(new StrokeComponent())
        rectangleShape.addComponent(new FillColorComponent())

        let fillComponent = rectangleShape.getComponentByTypeName("FillColorComponent")
        fillComponent["fillColor"] = {red: 1.0, green: 0, blue: 1.0, alpha: 1.0}

        rectangleShape.setStartPoint(new Vector2(10, 10))
        rectangleShape.setEndPoint(new Vector2(100, 100))

        huahuoEngine.GetCurrentLayer().addShape(rectangleShape)
        animationPlayer.setFrameId(60)

        fillComponent["fillColor"] = {red: 0, green: 0, blue: 1.0, alpha: 1.0}

        rectangleShape.position = new paper.Point(0, 500.0)

        animationPlayer.startPlay()
    })
}

export {initTest}