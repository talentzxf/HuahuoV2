import "mocha"
import {assert} from "chai"
import {Vector2, mirrorPoint} from "hhcommoncomponents";

let eps = 0.001

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

        p1 = new Vector2(0,1)
        p2 = new Vector2(1,0)

        result = mirrorPoint(origin, p1, p2)
        assert(new Vector2(1,1).distance(result) <= eps)

        result = mirrorPoint(result, p1, p2)
        assert(new Vector2(0,0).distance(result) <= eps)

        p1 = new Vector2(0,0)
        p2 = new Vector2(1,1)
        result = mirrorPoint(new Vector2(1,0), p1, p2)
        assert( new Vector2(0,1).distance(result) <= eps)

        result = mirrorPoint(result, p1, p2)
        assert( new Vector2(1,0).distance(result) <= eps)

        result = mirrorPoint(new Vector2(-1,0), p1, p2)
        assert( new Vector2(0,-1).distance(result) <= eps)

        result = mirrorPoint(result, p1, p2)
        assert( new Vector2(-1,0).distance(result) <= eps)
    })
})