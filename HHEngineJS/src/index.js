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
import {Particles} from "./ParticleSystem/Particles";
import {registerCustomFieldContentDivGeneratorConstructor} from "./Components/AbstractComponent"
import {BaseShapeActions} from "./EventGraph/BaseShapeActions";
import {EventNode} from "./EventGraph/Nodes/EventNode";
import {ActionNode} from "./EventGraph/Nodes/ActionNode";
import {StarMirrorShapeJS} from "./Shapes/StarMirrorShapeJS";
import {Utils} from "./Shapes/Utils";
import {SetShapeDecorator} from "./Shapes/LoadShape";

import {isInheritedFromClzName} from "./CppClassObjectFactory";
import {LGraphCanvas, LiteGraph} from "litegraph.js";

// All non-default components
import {GeneratorComponent} from "./Components/GeneratorComponent";
import {CurveGrowthComponent} from "./Components/CurveGrowthComponent";
import {RadialGradientComponent} from "./Components/RadialGradientComponent";
import {FollowCurveComponent} from "./Components/FollowCurveComponent";
import {EventGraphComponent} from "./Components/EventGraphComponent";
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

if (!window["taichiInitBegun"]) {
    window["taichiInitBegun"] = true

//     // This is just because StackBlitz has some weird handling of external scripts.
//     // Normally, you would just use `<script src="https://unpkg.com/taichi.js/dist/taichi.umd.js"></script>` in the HTML
//     const script = document.createElement('script');
//     script.addEventListener('load', async function () {
//         await huahuoEngine.OnTaichiInit()
//     });
// // script.src = 'https://unpkg.com/taichi.js/dist/taichi.umd.js';
//     script.src = 'https://unpkg.com/taichi.js/dist/taichi.dev.umd.js';
//
// // Append to the `head` element
//     document.head.appendChild(script);

    huahuoEngine.OnTaichiInit()
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
    Particles,
    NailComponent,
    EventGraphComponent,
    ColorStop,
    getNailManager,
    isInheritedFromClzName,
    registerCustomFieldContentDivGeneratorConstructor,
    BaseShapeActions,
    EventNode,
    ActionNode,
    LGraphCanvas,
    LiteGraph,
    StarMirrorShapeJS,
    Utils
}