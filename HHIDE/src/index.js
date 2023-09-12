import {HHContainer} from "hhpanel";
import {HHPanel} from 'hhpanel'
import {HHContent} from 'hhpanel'
import {HHTitle} from "hhpanel";
import {HHTimeline} from "hhtimeline";
import {NavTree} from "./UIComponents/NavTree"
import {DrawToolBar} from "./UIComponents/DrawToolBar";
import {SceneView} from "./SceneView/SceneView";
import {Inspector} from "./Inspector/Inspector";
import {BaseForm} from "./UIComponents/BaseForm"

import {RegisterForm} from "./Identity/RegisterForm";
import {UserInfoBar} from "./Identity/UserInfoBar";
import {PlayerController} from "./AnimationPlayer/PlayerController";
import {binaryFileUploader} from "./RESTApis/BinaryFileUploader";
import {HHToolBar} from "./UIComponents/ToolBar";
import {ProjectListForm} from "./UIComponents/ProjectListForm";
import {ProjectInfoForm} from "./UIComponents/ProjectInfoForm";
import {HHEditorToolBar} from "./UIComponents/HHEditorToolBar";
import {HHHistoryCommandList} from "./UIComponents/HHHistoryCommandList"
import {EventGraphEditorX} from "./InspectorX/CustomFieldReactGenerator/EventGraphEditorX"
import {SelectIconForm} from "./Inspector/CustomFieldDivGenerators/SelectIconForm";
import {EventNode} from "../../HHEngineJS/src/EventGraph/Nodes/EventNode";
import {ActionNode} from "../../HHEngineJS/src/EventGraph/Nodes/ActionNode";
import {FocusSceneViewCommand} from "./RedoUndo/FocusSceneViewCommand";
import {RemoveComponentCommand} from "./RedoUndo/RemoveComponentCommand"

import "./index.css"

// Input components
import {HHRefreshableDiv} from "./Inspector/InputComponents/HHRefreshableDiv";

import {library, dom} from "@fortawesome/fontawesome-svg-core";
import {faPlus} from "@fortawesome/free-solid-svg-icons/faPlus";
import {faMinus} from "@fortawesome/free-solid-svg-icons/faMinus"
import {faSlash} from "@fortawesome/free-solid-svg-icons/faSlash";
import {faCircle} from "@fortawesome/free-solid-svg-icons/faCircle";
import {faCircleXmark} from "@fortawesome/free-regular-svg-icons";
import {faBezierCurve} from "@fortawesome/free-solid-svg-icons/faBezierCurve";
import {faArrowPointer} from "@fortawesome/free-solid-svg-icons";
import {faSquare} from "@fortawesome/free-solid-svg-icons";
import {faFileAudio} from "@fortawesome/free-regular-svg-icons";
import {faEye} from "@fortawesome/free-regular-svg-icons"
import {faEyeSlash} from "@fortawesome/free-regular-svg-icons";
import {faEdit} from "@fortawesome/free-regular-svg-icons";
import {faFileImage} from "@fortawesome/free-regular-svg-icons";
import {faTimesCircle} from "@fortawesome/free-regular-svg-icons";
import {faShapes} from "@fortawesome/free-solid-svg-icons";
import {init} from "./init"

import "./i18nInit"
import {EditorShapeProxy} from "./ShapeDrawers/EditorShapeProxy";

library.add(faMinus)
library.add(faPlus)
library.add(faSlash)
library.add(faCircle)
library.add(faBezierCurve)
library.add(faArrowPointer)
library.add(faSquare)
library.add(faFileAudio)
library.add(faEye)
library.add(faEyeSlash)
library.add(faEdit)
library.add(faFileImage)
library.add(faTimesCircle)
library.add(faCircleXmark)
library.add(faShapes)
dom.watch();

init()
