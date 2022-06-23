import * as paper from "paper"

function pointsNear(p1:paper.Point, p2:paper.Point, margin:number){
    return p1.getDistance(p2) < margin
}

function relaxRectangle(rectangle, margin) {
    let retRectangle = rectangle.clone()
    retRectangle.x -= margin
    retRectangle.y -= margin
    retRectangle.width += 2 * margin
    retRectangle.height += 2 * margin

    return retRectangle
}
export {pointsNear,relaxRectangle}