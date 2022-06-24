import {LineDrawer} from "./LineDrawer";
import {CircleDrawer} from "./CircleDrawer";
import {CurveDrawer} from "./CurveDrawer";
import {RectangleDrawer} from "./RectangleDrawer"
import {ShapeSelector} from "./ShapeSelector";

let shapes = [
    new ShapeSelector(),
    new LineDrawer(),
    new CircleDrawer(),
    new RectangleDrawer(),
    new CurveDrawer()
]

let defaultShapeDrawerArray = shapes.filter(drawer=>{
    if(drawer.isDefaultDrawer()) return true
    return false
})

let defaultShapeDrawer = defaultShapeDrawerArray.length == 0 ? null: defaultShapeDrawerArray[0]

export {shapes, defaultShapeDrawer}