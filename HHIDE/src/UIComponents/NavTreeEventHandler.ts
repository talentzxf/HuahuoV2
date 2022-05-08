import {NavTree} from "./NavTree";
import {EngineAPI} from "../EngineAPI";
declare var ScriptEventHandlerImpl:any

class NavTreeEventHandler{
    private tree:NavTree

    public constructor(tree:NavTree) {
        this.tree = tree

        let handler = new ScriptEventHandlerImpl();
        handler.handleEvent = this.handleEvent.bind(this)
        EngineAPI.GetInstance().RegisterEvent("OnHierarchyChange", handler)
    }

    handleEvent(){
        this.tree.clearNodes()
        // EngineAPI.GetInstance().
    }
}

export {NavTreeEventHandler}