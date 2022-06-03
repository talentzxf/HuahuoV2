import {RenderEnginePaperJs} from "./RenderEngine/RenderEnginePaperImpl";
import {LineShape} from "./Shapes/LineShape"
import {ShapeStoreManager, ShapeStore, Layer} from "./ShapeStore/ShapeStore";
import {RenderEngine2D} from "./RenderEngine/RenderEngine2D";

let renderEngine2D : RenderEngine2D = new RenderEnginePaperJs()
export {renderEngine2D, LineShape, ShapeStoreManager, ShapeStore, Layer}