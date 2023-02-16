import {LineDrawer} from "./LineDrawer";
import {CircleDrawer} from "./CircleDrawer";
import {CurveDrawer} from "./CurveDrawer";
import {RectangleDrawer} from "./RectangleDrawer"
import {ShapeSelector} from "./ShapeSelector";
import {AudioClipSelector} from "./AudioClipSelector";
import {TextDrawer} from "./TextDrawer";
import {ImageSelector} from "./ImageSelector"
import {MirrorDrawer} from "./MirrorDrawer";
import {IconShapeDrawer} from "./IconShapeDrawer";
import {NailDrawer} from "./NailDrawer";
import {ParticleSystemDrawer} from "./ParticleSystemDrawer";

let shapes = [
    new ShapeSelector(),
    new LineDrawer(),
    new CircleDrawer(),
    new RectangleDrawer(),
    new CurveDrawer(),
    new TextDrawer(),
    new MirrorDrawer(),
    new AudioClipSelector(),
    new ImageSelector(),
    new IconShapeDrawer(),
    new NailDrawer(),
    new ParticleSystemDrawer()
]

let defaultShapeDrawerArray = shapes.filter(drawer=>{
    if(drawer.isDefaultDrawer()) return true
    return false
})

let defaultShapeDrawer = defaultShapeDrawerArray.length == 0 ? null: defaultShapeDrawerArray[0]

export {shapes, defaultShapeDrawer}