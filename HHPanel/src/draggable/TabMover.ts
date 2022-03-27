import {Vector2D} from "../math/Vector2D";
import {ResponsibleChain, ChainCallback} from "./ResponsibleChain";
import {HHTitle} from "../HHTitle";
import {OccupiedTitleManager} from "./OccupiedTitleManager";

class TabMoveParam {
    public ele: HHTitle
    public targetPos: Vector2D

    public constructor(inEle: HHTitle, inTargetPos: Vector2D) {
        this.ele = inEle
        this.targetPos = inTargetPos
    }
}

class TabMover{
    private static instance: TabMover;

    private handlerChain: ResponsibleChain<TabMoveParam> = new ResponsibleChain<TabMoveParam>();

    private constructor() {
        // Default behaviour, drag out of all the title bar holders.
        this.AddBack(this.DefaultTitleMoving)
    }

    public static getInstance(): TabMover{
        if(!TabMover.instance)
            TabMover.instance = new TabMover()

        return TabMover.instance
    }

    public DefaultTitleMoving(param: any):boolean{
        // If we are here, the title didn't belong to any other tab groups. Should clear it's occupied slots.
        OccupiedTitleManager.getInstance().Clear()
        param.ele.setScrPos(param.targetPos.X, param.targetPos.Y)
        return true
    }

    public TryMove(obj: any, inTargetPos: Vector2D){
        this.handlerChain.execute(new TabMoveParam(obj,inTargetPos))
    }

    public AddBack(lastCallBack: ChainCallback<TabMoveParam>):void {
        this.handlerChain.AddBack(lastCallBack)
    }

    public AddFront(frontCallBack: ChainCallback<TabMoveParam>):void{
        this.handlerChain.AddFront(frontCallBack)
    }
}

export {TabMover, TabMoveParam}