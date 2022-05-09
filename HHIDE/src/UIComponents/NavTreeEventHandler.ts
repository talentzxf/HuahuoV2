import {NavTree, TreeNode} from "./NavTree";
import {EngineAPI} from "../EngineAPI";
import {IsValidWrappedObject} from "../Utilities/WrappedObjectUtils"

declare var ScriptEventHandlerImpl: any
declare var TransformHierarchyEventArgs: any

class NavTreeEventHandler {
    private tree: NavTree
    private transformNodeMap: Map<number, TreeNode> = new Map();

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

    treeNodeExists(transform){
        return this.transformNodeMap.has(transform.ptr)
    }

    getOrCreateTreeNode(transform){
        let cppPtr = transform.ptr
        if(!this.transformNodeMap.has(cppPtr)){
            let newNode = new TreeNode(transform.GetName())
            this.transformNodeMap.set(cppPtr, newNode)
            return newNode
        }
        return this.transformNodeMap.get(cppPtr)
    }

    handleEvent(argsPointer) {
        let args = this.constructArgsFromPointer(argsPointer)

        let transform = args.GetTransform();

        if(!IsValidWrappedObject(transform.GetParent())){ // No Parent means this is the root object of the Scene
            if(!this.treeNodeExists(transform)){
                this.tree.appendTreeNode(this.tree.getRootTreeNode(), this.getOrCreateTreeNode(transform))
            }
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