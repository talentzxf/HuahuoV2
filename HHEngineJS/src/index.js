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
import {BaseShapeActor} from "./EventGraph/BaseShapeActor";
import {VariableNode} from "./EventGraph/UtilityNodes/VariableNode";
import {CompareNode} from "./EventGraph/UtilityNodes/CompareNode"
import {GetShapeComponentNode} from "./EventGraph/Nodes/GetShapeComponentNode";
import {SetComponentPropertyNode} from "./EventGraph/Nodes/SetComponentPropertyNode"
import {EventNode} from "./EventGraph/Nodes/EventNode";
import {ActionNode} from "./EventGraph/Nodes/ActionNode";
import {Vec2MathOperationNode} from "./EventGraph/UtilityNodes/Vec2MathOperationNode";
import {Vec2MathNumberMultiply} from "./EventGraph/UtilityNodes/Vec2MathNumberMultiply";
import {SetCursorShapeNode} from "./EventGraph/UtilityNodes/SetCursorShapeNode";
import {Vector2NumberNode} from "./EventGraph/UtilityNodes/Vector2NumberNode";
import {ConstVec2Node} from "./EventGraph/UtilityNodes/ConstVec2Node";
import {SwitchNode} from "./EventGraph/UtilityNodes/SwitchNode";
import {PlayerActions} from "./Player/PlayerActions";
import {NodeTargetType} from "./EventGraph/GraphActions";

import {StarMirrorShapeJS} from "./Shapes/StarMirrorShapeJS";
import {Utils} from "./Shapes/Utils";
import {AbstractComponent} from "./Components/AbstractComponent";
import {PropertyCategory, PropertyDef} from "./Components/PropertySheetBuilder"
import {getLiteGraphTypeFromPropertyType} from "./EventGraph/GraphUtils"

import {isInheritedFromClzName} from "./CppClassObjectFactory";
import {LGraphCanvas, LiteGraph} from "litegraph.js";

// All non-default components
import {CurveGrowthComponent} from "./Components/CurveGrowthComponent";
import {EventGraphComponent} from "./Components/EventGraphComponent";
import {Motor} from "./Components/Motor";
import {GeneratorComponent} from "./Components/GeneratorComponent";
import {ObjectGenerator} from "./Components/ObjectGenerator";
import {NailComponent} from "./Components/NailComponent";
import {RadialGradientComponent} from "./Components/RadialGradientComponent"
import {FollowCurveComponent} from "./Components/FollowCurveComponent";
import {RigidBody} from "./Components/Physics/RigidBody";
import {Camera2D} from "./Components/Camera2D";
import {ImageSpriteController} from "./Components/ImageSpriteController";
import {renderEngine2D} from "./RenderEngine/RenderEnginePaperImpl"

import "./Shapes/LoadShape"

import {ValueChangeHandler} from "./Shapes/ValueChangeHandler";
import * as paper from "paper"


function InitWASM() {
    if (typeof Module != 'undefined') {
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
    } else {
        console.log("Module is null?? Init in next tick!")
        setTimeout(InitWASM, 0)
    }
}

InitWASM()


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
    Camera2D,
    NailComponent,
    EventGraphComponent,
    CurveGrowthComponent,
    FollowCurveComponent,
    RadialGradientComponent,
    GeneratorComponent,
    ImageSpriteController,
    RigidBody,
    Motor,
    ObjectGenerator,
    ColorStop,
    getNailManager,
    isInheritedFromClzName,
    BaseShapeActor,
    EventNode,
    ActionNode,
    SwitchNode,
    VariableNode,
    CompareNode,
    GetShapeComponentNode,
    SetComponentPropertyNode,
    Vec2MathOperationNode,
    Vec2MathNumberMultiply,
    ConstVec2Node,
    Vector2NumberNode,
    SetCursorShapeNode,
    LGraphCanvas,
    LiteGraph,
    StarMirrorShapeJS,
    Utils,
    AbstractComponent,
    PropertyCategory,
    PropertyDef,
    getLiteGraphTypeFromPropertyType,
    PlayerActions,
    NodeTargetType,

}