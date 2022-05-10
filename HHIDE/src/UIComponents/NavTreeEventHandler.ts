import {NavTree, TreeNode} from "./NavTree";
import {EngineAPI} from "../EngineAPI";
import {IsValidWrappedObject} from "../Utilities/WrappedObjectUtils"
import {GameObjectManager} from "../HuaHuoEngine/GameObjectManager";

declare var ScriptEventHandlerImpl: any
declare var TransformHierarchyEventArgs: any
declare var GameObject: any
declare var Transform: any

class NavTreeEventHandler {
    private tree: NavTree
    private transformNodeMap: Map<number, TreeNode> = new Map();
    private nodeTransformMap: Map<TreeNode, number> = new Map()

    public constructor(tree: NavTree) {
        this.tree = tree

        let handler = new ScriptEventHandlerImpl();
        handler.handleEvent = this.handleOnHierarchyChange.bind(this)
        EngineAPI.GetInstance().RegisterEvent("OnHierarchyChange", handler)

        let setParentHandler = new ScriptEventHandlerImpl();
        setParentHandler.handleEvent = this.handleOnHierarchyChangeSetParent.bind(this)
        EngineAPI.GetInstance().RegisterEvent("OnHierarchyChangedSetParent", setParentHandler)
    }

    convertObjectFromPointer(transformPtr, classType){
        let returnArgs = Module["wrapPointer"](transformPtr)
        returnArgs.__proto__ = classType.prototype
        return returnArgs
    }

    treeNodeExists(transform){
        return this.transformNodeMap.has(transform.ptr)
    }

    createEmptyGameObject() {
        let treeNode = this.tree.getSelectedTreeNode()

        let gameObject:any = GameObjectManager.getInstance().createGameObject();
        if(treeNode){
            let parentTransform = this.convertObjectFromPointer( this.nodeTransformMap.get(treeNode) , Transform)
            if(IsValidWrappedObject(parentTransform))
                gameObject.GetTransform().SetParent(parentTransform)
        }
    }

    getOrCreateTreeNode(transform){
        let cppPtr = transform.ptr
        if(!this.transformNodeMap.has(cppPtr)){
            let newNode = new TreeNode(transform.GetName())
            this.transformNodeMap.set(cppPtr, newNode)
            this.nodeTransformMap.set(newNode, cppPtr)
            return newNode
        }
        return this.transformNodeMap.get(cppPtr)
    }

    handleOnHierarchyChangeSetParent(argsPointer){
        let args = this.convertObjectFromPointer(argsPointer, TransformHierarchyEventArgs)
        let oldTreeNode = this.transformNodeMap.get(args.GetOldParent().ptr)
        let newTreeNode = this.transformNodeMap.get(args.GetNewParent().ptr)
        let curNode = this.transformNodeMap.get(args.GetTransform().ptr)

        this.tree.moveNode(curNode, oldTreeNode, newTreeNode)
    }

    handleOnHierarchyChange(argsPointer) {
        let args = this.convertObjectFromPointer(argsPointer, TransformHierarchyEventArgs)

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