import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector:"hh-refreshable-div"
})
class HHRefreshableDiv extends HTMLElement implements RefreshableComponent{
    refresh(){
        // Refresh all elements that contain a refresh function
        let nodeStack: Array<Element> = new Array<Element>()
        nodeStack.push(this)

        while(nodeStack.length > 0){
            let curNode = nodeStack.pop()
            for(let child of curNode.children){
                nodeStack.push(child)
            }

            if(curNode != this){
                if(curNode["refresh"] && typeof curNode["refresh"] == "function"){
                    curNode["refresh"]()
                }
            }
        }
    }
}

export {HHRefreshableDiv}