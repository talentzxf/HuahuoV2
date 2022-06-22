import * as paper from "paper"

function pointsNear(p1:paper.Point, p2:paper.Point, margin:number){
    return p1.getDistance(p2) < margin
}

export {pointsNear}