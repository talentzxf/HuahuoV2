import {RenderEnginePaperJs} from "./RenderEngine/RenderEnginePaperImpl";
import {LineShapeJS} from "./Shapes/LineShapeJS"
import {CircleShapeJS} from "./Shapes/CircleShapeJS"
import {RectangleJS} from "./Shapes/RectangleJS";
import {ImageShapeJS} from "./Shapes/ImageShapeJS";
import {huahuoEngine} from "./EngineAPI";
import {GlobalConfig} from "./GlobalConfig";
import {AudioShapeJS} from "./Shapes/AudioShapeJS";
import {ElementShapeJS} from "./Shapes/ElementShapeJS";
import {Player} from "./Player/Player"
import {BaseShapeJS} from "./Shapes/BaseShapeJS";
import {TextShapeJS} from "./Shapes/TextShapeJS";
import {CurveShapeJS} from "./Shapes/CurveShapeJS"
import {SVGShapeJS} from "./Shapes/SVGShapeJS";
import {MirrorShapeJS} from "./Shapes/MirrorShapeJS";
import {ColorStop} from "./Components/ColorStop";
import {getNailManager} from "./IK/GetNailManager";
import {NailShapeJS} from "./Shapes/NailShapeJS";
import {ParticleSystemJS} from "./Shapes/ParticleSystemJS";
import {ParticleSystemRenderer} from "./Components/ParticleSystemRenderer";

import {isInheritedFromClzName} from "./CppClassObjectFactory";

// All non-default components
import {GeneratorComponent} from "./Components/GeneratorComponent";
import {CurveGrowthComponent} from "./Components/CurveGrowthComponent";
import {RadialGradientComponent} from "./Components/RadialGradientComponent";
import {NailComponent} from "./Components/NailComponent";

import "./Shapes/LoadShape"

import {ValueChangeHandler} from "./Shapes/ValueChangeHandler";
import * as paper from "paper"

let renderEngine2D = new RenderEnginePaperJs()

if (Module.IsWASMInited && Module.IsWASMInited()) {
    console.log("Init right now")
    huahuoEngine.OnInit()
} else {
    console.log("Init later")
    Module.onRuntimeInitialized = () => {
        console.log("Init now!")
        huahuoEngine.OnInit()
    }
}

window.enginejsInited = true
export {
    renderEngine2D,
    ValueChangeHandler,
    LineShapeJS,
    CircleShapeJS,
    RectangleJS,
    ImageShapeJS,
    TextShapeJS,
    AudioShapeJS,
    ElementShapeJS,
    huahuoEngine,
    paper,
    GlobalConfig,
    Player,
    BaseShapeJS,
    CurveShapeJS,
    SVGShapeJS,
    MirrorShapeJS,
    NailShapeJS,
    ParticleSystemJS,
    ParticleSystemRenderer,
    NailComponent,
    ColorStop,
    getNailManager,
    isInheritedFromClzName
}