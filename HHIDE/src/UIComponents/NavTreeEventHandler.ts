import {NavTree, TreeNode} from "./NavTree";
import {EngineAPI} from "../EngineAPI";

declare var ScriptEventHandlerImpl: any
declare var SceneRootTransformArray: any

class NavTreeEventHandler {
    private tree: NavTree

    public constructor(tree: NavTree) {
        this.tree = tree

        let handler = new ScriptEventHandlerImpl();
        handler.handleEvent = this.handleEvent.bind(this)
        EngineAPI.GetInstance().RegisterEvent("OnHierarchyChange", handler)
    }

    handleEvent() {
        this.tree.clearNodes()

        let rootTreeNode = this.tree.getRootTreeNode();

        let scene = EngineAPI.GetInstance().GetSceneManager().GetActiveScene();
        if (!scene.IsEmpty()) {
            let rootTransformArray = new SceneRootTransformArray(scene)
            do {
                let curTransform = rootTransformArray.GetCurrentTransform();
                rootTreeNode.appendChild(new TreeNode(curTransform.GetName()))
            } while (rootTransformArray.MoveNext());
            this.tree.attachRootNode()
        }
    }
}

export {NavTreeEventHandler}