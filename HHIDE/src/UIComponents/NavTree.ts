// import {ContextMenu, CustomElement} from "hhcommoncomponents";
// import {HHPanel} from "hhpanel";
// import "/css/navtree.css"
// import {NavTreeEventHandler} from "./NavTreeEventHandler";
// import {EngineAPI} from "../../../HHEngineJS/src/EngineAPI";
//
// class TreeNode {
//     private name: String
//     private children: Array<TreeNode> = new Array();
//     private htmlElement: HTMLElement
//
//     private openButton: HTMLButtonElement
//     private closeButton: HTMLButtonElement
//
//     constructor(name: String) {
//         this.name = name;
//     }
//
//     getName() {
//         return this.name;
//     }
//
//     appendChild(node: TreeNode) {
//         this.children.push(node)
//     }
//
//     removeChild(node: TreeNode) {
//         this.children.forEach((item, index, array) => {
//             if (item == node) array.splice(index, 1)
//         })
//     }
//
//     getChildren(): Array<TreeNode> {
//         return this.children;
//     }
//
//     getChild(idx: number): TreeNode {
//         return this.children[idx];
//     }
//
//     isLeaf(): Boolean {
//         return this.children.length == 0;
//     }
//
//     setHTMLElement(element: HTMLElement) {
//         this.htmlElement = element
//
//         if (this.htmlElement) {
//             this.openButton = this.htmlElement.querySelector<HTMLButtonElement>("#open-button")
//             this.closeButton = this.htmlElement.querySelector<HTMLButtonElement>("#close-button")
//
//             if (!this.isLeaf()) {
//                 this.showOpenButton()
//             }
//         }
//     }
//
//     showOpenButton() {
//         this.openButton.style.display = "inline"
//         this.closeButton.style.display = "none"
//     }
//
//     showCloseButton() {
//         this.openButton.style.display = "none"
//         this.closeButton.style.display = "inline"
//     }
//
//     getHTMLElement(): HTMLElement {
//         return this.htmlElement
//     }
// }
//
// let childrenHolder: string = "children-holder"
// let childrenHolderId: string = "#" + childrenHolder
//
// @CustomElement({
//     selector: "hh-navtree",
// })
// class NavTree extends HTMLElement {
//     private rootNode: TreeNode = new TreeNode("SceneRoot");
//     private domRoot: HTMLElement = document.createElement("div");
//     private contextMenu: ContextMenu = new ContextMenu()
//     private currentSelectedDiv: HTMLElement;
//
//     private treeEventHandler: NavTreeEventHandler;
//
//     private spanNodeMap: Map<HTMLElement, TreeNode>
//
//     private selectedTreeNode: TreeNode
//
//     constructor() {
//         super();
//
//         // this.rootNode.appendChild(new TreeNode("Child1"))
//         // this.rootNode.appendChild(new TreeNode("Child2"))
//         //
//         // this.rootNode.getChild(0).appendChild(new TreeNode("Child1-1"))
//         // this.rootNode.getChild(1).appendChild(new TreeNode("Child1-1"))
//
//         let _this = this
//         EngineAPI.ExecuteAfterInited(function () {
//             _this.treeEventHandler = new NavTreeEventHandler(_this)
//             _this.contextMenu.setItems([
//                 {
//                     itemName: "Create Empty Game Object",
//                     onclick: _this.treeEventHandler.createEmptyGameObject.bind(_this.treeEventHandler)
//                 },
//                 {
//                     itemName: "Create Cube",
//                     onclick: _this.treeEventHandler.createCube.bind(_this.treeEventHandler)
//                 },
//                 {
//                     itemName: "Delete Object",
//                     onclick: () => {
//                         alert("Delete object")
//                     }
//                 }
//             ])
//         })
//
//         this.spanNodeMap = new Map<HTMLElement, TreeNode>();
//     }
//
//     public getRootTreeNode(): TreeNode {
//         return this.rootNode;
//     }
//
//     moveNode(curNode: TreeNode, oldParent: TreeNode, newParent: TreeNode) {
//         if (newParent === oldParent) {
//             return;
//         }
//
//         if (!oldParent) {
//             oldParent = this.rootNode
//         }
//
//         oldParent.removeChild(curNode)
//         let oldParentChildrenHolder = oldParent.getHTMLElement().querySelector(childrenHolderId)
//         oldParentChildrenHolder.removeChild(curNode.getHTMLElement())
//
//         this.appendTreeNode(newParent, curNode)
//     }
//
//     clearNodes() {
//         if (this.rootNode.getHTMLElement()) {
//             this.domRoot.removeChild(this.rootNode.getHTMLElement())
//         }
//         this.rootNode = new TreeNode("SceneRoot")
//     }
//
//     selectItem(targetDiv: HTMLElement) {
//         let _this = this
//         return function (evt: MouseEvent) {
//             evt.stopPropagation()
//
//             if (_this.currentSelectedDiv) {
//                 _this.currentSelectedDiv.setAttribute("selected", "false")
//             }
//
//             _this.currentSelectedDiv = targetDiv;
//             targetDiv.setAttribute("selected", "true")
//             _this.selectedTreeNode = _this.spanNodeMap.get(targetDiv)
//         }
//     }
//
//     getOrCreateNode(treeNode: TreeNode) {
//         let _this = this
//
//         let div = treeNode.getHTMLElement();
//
//         let span = document.createElement("span")
//         this.spanNodeMap.set(span, treeNode)
//
//         span.innerHTML += treeNode.getName()
//         if (div == null) {
//             div = document.createElement("div");
//             div.className = "navtree-item"
//
//             if (!treeNode.isLeaf()) {
//                 let openButton = document.createElement("button")
//                 openButton.id = "open-button"
//                 openButton.innerHTML = "<i class='fas fa-plus'/></button>"
//                 div.appendChild(openButton)
//
//                 let closeButton = document.createElement("button")
//                 closeButton.id = "close-button"
//                 closeButton.style.display = 'none'
//                 closeButton.innerHTML = "<i class='fas fa-minus'/></button>"
//                 div.appendChild(closeButton)
//
//                 let childrenDiv = document.createElement("div")
//                 childrenDiv.id = childrenHolder
//                 childrenDiv.style.display = "inline"
//
//                 openButton.addEventListener('click', _this.expandTreeNode(treeNode, childrenDiv))
//                 closeButton.addEventListener('click', _this.closeTreeNode(treeNode, childrenDiv))
//
//                 div.appendChild(span)  // Show Name
//
//                 let childSuperDiv = document.createElement("div")
//                 childSuperDiv.id = "children-superdiv"
//                 childSuperDiv.style.display = "flex"
//                 childSuperDiv.style.flexDirection = "row"
//                 let placeHolder = document.createElement("span")
//                 placeHolder.style.width = "10px"
//
//                 childSuperDiv.appendChild(placeHolder)
//                 childSuperDiv.appendChild(childrenDiv)
//                 div.appendChild(childSuperDiv)
//             } else {
//                 div.appendChild(span)  // Show Name
//             }
//
//             div.addEventListener("click", this.selectItem(span).bind(_this))
//
//             treeNode.setHTMLElement(div)
//         }
//         return div;
//     }
//
//     closeTreeNode(node: TreeNode, div: HTMLElement) {
//         return function () {
//             if (node.getHTMLElement() != null) {
//                 div.style.display = "none"
//                 node.showOpenButton()
//             }
//         }
//     }
//
//     expandTreeNode(node: TreeNode, div: HTMLElement) {
//         let _this = this
//         return function () {
//             div.style.display = "inline"
//             node.showCloseButton()
//
//             node.getChildren().forEach(child => {
//                 if (node.getHTMLElement() != this) {
//                     let childDiv = _this.getOrCreateNode(child)
//                     div.appendChild(childDiv)
//                 }
//             })
//         }
//     }
//
//     getSelectedTreeNode() {
//         return this.selectedTreeNode
//     }
//
//     findParentPanel() {
//         let candidate = this.parentElement
//         while (candidate != null) {
//             if (candidate instanceof HHPanel) {
//                 return candidate
//             }
//             candidate = candidate.parentElement
//         }
//
//         return null
//     }
//
//     connectedCallback() {
//         this.domRoot.className = "navtree"
//         let parentPanel: HTMLElement = this.findParentPanel();
//         parentPanel.oncontextmenu = this.contextMenu.onContextMenu.bind(this.contextMenu);
//         this.append(this.domRoot)
//
//         this.attachRootNode()
//     }
//
//     appendTreeNode(parent: TreeNode, newChild: TreeNode) {
//         let isParentLeaf = parent.isLeaf();
//
//         parent.appendChild(newChild)
//         if (parent.getHTMLElement() == null) {
//             throw "Parent HTMLElement has not been inited yet!"
//         }
//
//         let childrenHolderDiv
//         let parentHtmlElement
//
//         if (isParentLeaf) {
//             let previousHTMLElement = parent.getHTMLElement()
//             let previousParentParent = previousHTMLElement.parentElement
//
//             parent.setHTMLElement(null)
//             parentHtmlElement = this.getOrCreateNode(parent)
//
//             previousParentParent.insertBefore(parentHtmlElement, previousHTMLElement)
//             previousParentParent.removeChild(previousHTMLElement)
//             previousParentParent.appendChild(parentHtmlElement)
//             childrenHolderDiv = parentHtmlElement.querySelector(childrenHolderId)
//         } else {
//             parentHtmlElement = parent.getHTMLElement()
//             childrenHolderDiv = parentHtmlElement.querySelector(childrenHolderId)
//             childrenHolderDiv.appendChild(this.getOrCreateNode(newChild))
//         }
//
//         this.expandTreeNode(parent, childrenHolderDiv)()
//     }
//
//     private attachRootNode() {
//         this.domRoot.appendChild(this.getOrCreateNode(this.rootNode))
//     }
// }
//
// export {NavTree, TreeNode}