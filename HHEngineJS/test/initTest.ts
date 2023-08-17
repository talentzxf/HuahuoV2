import {huahuoEngine, Player, RectangleJS, renderEngine2D} from "../src";
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

class BulletTest {
    m_world: b2World
    m_body: b2Body
    m_x: number
    m_bullet: b2Body

    constructor() {
        // Test box2D engine.
        this.m_world = b2World.Create({x: 0, y: 10})

        {
            const body = this.m_world.CreateBody();

            const edge = new b2EdgeShape();

            edge.SetTwoSided(new b2Vec2(0, 500), new b2Vec2(1000, 500));
            body.CreateFixture({shape: edge});

            const shape = new b2PolygonShape();
            shape.SetAsBox(0.2, 1, new b2Vec2(0.5, 800), 0);
            body.CreateFixture({shape});
        }


        {
            const box = new b2PolygonShape();
            box.SetAsBox(2, 0.1);

            this.m_body = this.m_world.CreateBody({
                type: b2BodyType.b2_dynamicBody,
                position: {
                    x: 0,
                    y: 4,
                },
            });
            this.m_body.CreateFixture({shape: box, density: 1});

            box.SetAsBox(0.25, 0.25);

            // this.m_x = b2RandomFloat(-1, 1);
            this.m_x = 0.20352793;

            this.m_bullet = this.m_world.CreateBody({
                type: b2BodyType.b2_dynamicBody,
                // bullet: true,
                position: {
                    x: 100,
                    y: 10,
                },
            });
            this.m_bullet.CreateFixture({shape: box, density: 100});

            // this.m_bullet.SetLinearVelocity(new b2Vec2(0, -50));
        }
    }

    Launch() {
        this.m_body.SetTransformVec(new b2Vec2(0, 4), 0);
        this.m_body.SetLinearVelocity(b2Vec2.ZERO);
        this.m_body.SetAngularVelocity(0);

        this.m_x = b2RandomFloat(-1, 1);
        this.m_bullet.SetTransformVec(new b2Vec2(100, 10), 0);
        // this.m_bullet.SetLinearVelocity(new b2Vec2(0, -50));
        this.m_bullet.SetAngularVelocity(0);

        b2Gjk.reset();
        b2Toi.reset();
    }

    Step(){
        this.m_world.Step( 1/30, {
            velocityIterations: 8,
            positionIterations: 3
        });
    }

    Draw(){
        for(let b = this.m_world.GetBodyList(); b; b = b.GetNext()){
            let shape = b.GetUserData()
            if(shape){
                const xf = b.GetTransform();

                shape.position = new paper.Point(xf.GetPosition().x, xf.GetPosition().y)
            }
        }
    }
}

function initTest() {
    huahuoEngine.ExecuteAfterInited(() => {
        prepareEnvironment()

        animationPlayer.setFrameId(0)
        let rectangleShape: RectangleJS = new RectangleJS();
        rectangleShape.addComponent(new StrokeComponent())
        rectangleShape.addComponent(new FillColorComponent())

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

        let bulletTest = new BulletTest()
        bulletTest.m_bullet.SetUserData(rectangleShape)

        bulletTest.Launch()

        let stepFunction = ()=>{
            bulletTest.Step()
            bulletTest.Draw()

            requestAnimationFrame(stepFunction)
        }

        stepFunction()

    })
}

export {initTest}