import "mocha"
import {assert} from "chai"
import {Vector2, mirrorPoint} from "hhcommoncomponents";

describe('Test mirror', ()=> {
    it("Test mirror of points", ()=>{
        let origin = new Vector2(0,0)
        let p1 = new Vector2(0,0)
        let p2 = new Vector2(0,0)

        let result = mirrorPoint(origin, p1, p2)
        assert(result.x == 0.0 && result.y == 0.0)

        p1 = new Vector2(1,0)
        p2 = new Vector2(1,1)

        result = mirrorPoint(origin, p1, p2)
        assert(result.x == 2 && result.y == 0.0)

        p1 = new Vector2(0, 1)
        p2 = new Vector2(1,1)
        result = mirrorPoint(origin, p1, p2)
        assert(result.x == 0 && result.y == 2)

    })
})