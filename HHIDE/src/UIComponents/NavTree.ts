import {CustomElement} from "hhpanel";

console.log(CustomElement)

class TreeNode {
    private name: String
    private children: Array<TreeNode> = new Array();
    private htmlElement: HTMLElement

    private openButton: HTMLButtonElement
    private closeButton: HTMLButtonElement

    constructor(name: String) {
        this.name = name;
    }

    getName() {
        return this.name;
    }

    appendChild(node: TreeNode) {
        this.children.push(node)
    }

    getChildren(): Array<TreeNode> {
        return this.children;
    }

    getChild(idx: number): TreeNode {
        return this.children[idx];
    }

    isLeaf(): Boolean {
        return this.children.length == 0;
    }

    setHTMLElement(element: HTMLElement) {
        this.htmlElement = element
        this.openButton = this.htmlElement.querySelector<HTMLButtonElement>("#open-button")
        this.closeButton = this.htmlElement.querySelector<HTMLButtonElement>("#close-button")

        if(!this.isLeaf()) {
            this.showOpenButton()
        }

    }

    showOpenButton() {
        this.openButton.style.display = "inline"
        this.closeButton.style.display = "none"
    }

    showCloseButton() {
        this.openButton.style.display = "none"
        this.closeButton.style.display = "inline"
    }

    getHTMLElement(): HTMLElement {
        return this.htmlElement
    }
}

@CustomElement({
    selector: "hh-navtree",
})
class NavTree extends HTMLElement {
    private rootNode: TreeNode = new TreeNode("SceneRoot");
    private domRoot: HTMLElement = document.createElement("div");

    constructor() {
        super();

        this.rootNode.appendChild(new TreeNode("Child1"))
        this.rootNode.appendChild(new TreeNode("Child2"))

        this.rootNode.getChild(0).appendChild(new TreeNode("Child1-1"))
        this.rootNode.getChild(1).appendChild(new TreeNode("Child1-1"))
    }

    getOrCreateNode(treeNode: TreeNode) {
        let _this = this

        let div = treeNode.getHTMLElement();
        if (div == null) {
            div = document.createElement("div");

            if (!treeNode.isLeaf()) {
                let openButton = document.createElement("button")
                openButton.id = "open-button"
                openButton.innerHTML = "<i class='fas fa-plus'/></button>"
                div.appendChild(openButton)

                let closeButton = document.createElement("button")
                closeButton.id = "close-button"
                closeButton.style.display = 'none'
                closeButton.innerHTML = "<i class='fas fa-minus'/></button>"
                div.appendChild(closeButton)

                let childrenDiv = document.createElement("div")
                childrenDiv.id = "children-holder"
                childrenDiv.style.display = "inline"

                openButton.addEventListener('click', _this.expandTreeNode(treeNode, childrenDiv))
                closeButton.addEventListener('click', _this.closeTreeNode(treeNode, childrenDiv))

                let span = document.createElement("span")
                span.innerHTML += treeNode.getName()
                div.appendChild(span)  // Show Name

                let childSuperDiv = document.createElement("div")
                childSuperDiv.id = "children-superdiv"
                childSuperDiv.style.display = "flex"
                childSuperDiv.style.flexDirection = "row"
                let placeHolder = document.createElement("span")
                placeHolder.style.width = "10px"

                childSuperDiv.appendChild(placeHolder)
                childSuperDiv.appendChild(childrenDiv)
                div.appendChild(childSuperDiv)
            } else {
                let span = document.createElement("span")
                span.innerHTML += treeNode.getName()
                div.appendChild(span)  // Show Name
            }

            treeNode.setHTMLElement(div)
        }
        return div;
    }

    closeTreeNode(node: TreeNode, div: HTMLElement) {
        return function () {
            if (node.getHTMLElement() != null) {
                div.style.display = "none"
                node.showOpenButton()
            }
        }
    }

    expandTreeNode(node: TreeNode, div: HTMLElement) {
        let _this = this
        return function () {
            div.style.display = "inline"
            node.showCloseButton()

            node.getChildren().forEach(child => {
                if (node.getHTMLElement() != this) {
                    let childDiv = _this.getOrCreateNode(child)
                    div.appendChild(childDiv)
                }
            })
        }
    }

    connectedCallback() {
        this.append(this.domRoot)

        this.domRoot.appendChild(this.getOrCreateNode(this.rootNode))
    }
}

export {NavTree}