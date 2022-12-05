const penWidth = 10
const penHeight = 10
const penCapHeight = 10

const unselectedPenCapColor = new paper.Color("lightgray")
const selectedPenCapColor = new paper.Color("black")

function createPenShape(){
    let penGroup = new paper.Group()
    let penBody = new paper.Path.Rectangle(new paper.Point(0, 0), new paper.Point(penWidth, penHeight))

    let penCapSegments = [new paper.Point(0, 0), new paper.Point(penWidth / 2, -penCapHeight), new paper.Point(penWidth, 0)]
    penBody.fillColor = new paper.Color("red")
    penBody.strokeColor = new paper.Color("black")
    penBody.strokeWidth = 3
    penGroup.addChild(penBody)

    let penCap = new paper.Path(penCapSegments)
    penCap.closed = true
    penCap.fillColor = new paper.Color("lightgray")
    penCap.strokeColor = new paper.Color("black")
    penCap.strokeWidth = 3
    penGroup.addChild(penCap)

    return [penGroup, penBody, penCap]
}

export {createPenShape, selectedPenCapColor, unselectedPenCapColor}