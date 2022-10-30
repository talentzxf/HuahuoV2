import {Vector2} from "./Vector2";

// Mirror a point by a line.
function mirrorPoint(p: Vector2, l1: Vector2, l2: Vector2): Vector2 {
    let A = l2.y - l1.y
    let B = -(l2.x - l1.x)
    let C = -A * l1.x - B * l1.y

    if (B == 0) { // It's a vertical line. Mirror the x.
        if (A == 0) { // It's a vertical line crossing origin. Mirror x by y-axis
            return new Vector2(-p.x, p.y)
        }
        let interceptDistance = -C / A
        return new Vector2(2 * interceptDistance - p.x, p.y)

    } else {
        let M = Math.sqrt(A * A + B * B)
        let A_prim = A / M
        let B_prim = B / M
        let C_prim = C / M

        let D = A_prim * p.x + B_prim * p.y + C_prim

        let resX = p.x - 2 * A_prim * D
        let resY = p.y - 2 * B_prim * D

        return new Vector2(resX, resY)
    }
}

export {mirrorPoint}