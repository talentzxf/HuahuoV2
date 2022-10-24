import {AbstractComponent, interpolateValue} from "./AbstractComponent";

class CurveGrowthComponent extends AbstractComponent{
    @interpolateValue()
    growth: number;
}