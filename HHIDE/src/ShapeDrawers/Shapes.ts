import {LineDrawer} from "./LineDrawer";
import {CircleDrawer} from "./CircleDrawer";
import {CurveDrawer} from "./CurveDrawer";
import {ShapeSelector} from "./ShapeSelector";

let shapes = [
    new ShapeSelector(),
    new LineDrawer(),
    new CircleDrawer(),
    new CurveDrawer()
]

export {shapes}