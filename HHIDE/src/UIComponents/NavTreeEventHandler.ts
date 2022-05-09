import {NavTree, TreeNode} from "./NavTree";
import {EngineAPI} from "../EngineAPI";
import {IsValidWrappedObject} from "../Utilities/WrappedObjectUtils"

declare var ScriptEventHandlerImpl: any
declare var SceneRootTransformArray: any
declare var TransformHierarchyEventArgs: any

class NavTreeEventHandler {
    private tree: NavTree

    public constructor(tree: NavTree) {
        this.tree = tree

        let handler = new ScriptEventHandlerImpl();
        handler.handleEvent = this.handleEvent.bind(this)
        EngineAPI.GetInstance().RegisterEvent("OnHierarchyChange", handler)
    }

    constructArgsFromPointer(args){
        let returnArgs = Module["wrapPointer"](args)
        returnArgs.__proto__ = TransformHierarchyEventArgs.prototype
        return returnArgs
    }

    handleEvent(argsPointer) {
        let args = this.constructArgsFromPointer(argsPointer)

        let transform = args.GetTransform();

        if(!IsValidWrappedObject(transform.GetParent())){ // No Parent means this is the root object of the Scene
            this.tree.appendTreeNode(this.tree.getRootTreeNode(), new TreeNode(transform.GetName()))
        }

        // this.tree.clearNodes()
        //
        // let rootTreeNode = this.tree.getRootTreeNode();
        //
        // let scene = EngineAPI.GetInstance().GetSceneManager().GetActiveScene();
        // if (!scene.IsEmpty()) {
        //     let rootTransformArray = new SceneRootTransformArray(scene)
        //     do {
        //         let curTransform = rootTransformArray.GetCurrentTransform();
        //         rootTreeNode.appendChild(new TreeNode(curTransform.GetName()))
        //     } while (rootTransformArray.MoveNext());
        //     this.tree.attachRootNode()
        // }
    }
}

export {NavTreeEventHandler}