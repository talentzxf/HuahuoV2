import {AbstractComponent, staticProperty} from "./AbstractComponent";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

class MirrorComponent extends AbstractComponent{
    @staticProperty()
    targetShapeArray: Array<BaseShapeJS>

}