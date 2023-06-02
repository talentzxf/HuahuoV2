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
import {StarMirrorDrawer} from "./StarMirrorDrawer";
import huahuoProperties from "/dist/hhide.properties";

let shapes = [
    new ShapeSelector(),
    new LineDrawer(),
    new CircleDrawer(),
    new RectangleDrawer(),
    new CurveDrawer(),
    new TextDrawer(),
    new MirrorDrawer(),
    new StarMirrorDrawer(),
    new AudioClipSelector(),
    new ImageSelector(),
    new IconShapeDrawer(),
]

let experimentalShapes = [
    new NailDrawer(),
    new ParticleSystemDrawer()
]

if(huahuoProperties["huahuo.experimentalFeatures.enable"]){
    shapes = shapes.concat(experimentalShapes)
}

let defaultShapeDrawerArray = shapes.filter(drawer=>{
    if(drawer.isDefaultDrawer()) return true
    return false
})

let defaultShapeDrawer: ShapeSelector = defaultShapeDrawerArray.length == 0 ? null: defaultShapeDrawerArray[0] as ShapeSelector

export {shapes, defaultShapeDrawer}