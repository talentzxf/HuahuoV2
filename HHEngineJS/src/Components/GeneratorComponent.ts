import {AbstractComponent, Component, PropertyValue} from "./AbstractComponent";
import {PropertyCategory} from "./PropertySheetBuilder";
import * as paper from "paper";

//TODO: Move all these non default components into another sub-project
@Component()
class GeneratorComponent extends AbstractComponent{
    @PropertyValue(PropertyCategory.shapeArray)
    targetShapeArray

    paperShapeGroup: paper.Group
    constructor(rawObj?) {
        super(rawObj);

        this.paperShapeGroup = new paper.Group()
        this.paperShapeGroup.applyMatrix = false
        this.paperShapeGroup.data.meta = this.baseShape

        // let line1 = new paper.Path.Line(new paper.Point(0, 0), new paper.Point(1000, 0)) // X Axis
        // let line2 = new paper.Path.Line(new paper.Point(0, 0), new paper.Point(0, 1000)) // Y Axis
        //
        // line1.strokeColor = new paper.Color("red")
        // line1.strokeWidth = 1
        // line2.strokeColor = new paper.Color("green")
        // line2.strokeWidth = 1
        //
        // this.paperShapeGroup.addChild(line1)
        // this.paperShapeGroup.addChild(line2)
    }
}

export {GeneratorComponent}