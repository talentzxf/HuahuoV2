import {AbstractComponent} from "hhenginejs";
import {getMethodsAndVariables} from "hhcommoncomponents";

class ComponentProxyHandler{
    targetComponent: AbstractComponent

    functionMap: Map<string, Function> = new Map()

    proxy
    setProxy(proxy){
        this.proxy = proxy
    }

    constructor(targetComponent) {
        this.targetComponent = targetComponent

        let _this = this
        getMethodsAndVariables(this, true).forEach((key)=>{
            if(typeof _this[key] == "function" &&
                key != "constructor" &&
                !key.startsWith("__") &&
                key != "get"){
                _this.functionMap.set(key, _this[key].bind(_this))
            }
        })
    }

    get(target, propKey, receiver){
        const origProperty = target[propKey]

        let _this = this

        if(origProperty instanceof Function){
            return function(...args){
                if(!_this.functionMap.has(origProperty.name)){
                    return origProperty.apply(this, args)
                }

                return _this.functionMap.get(origProperty.name).apply(this, args)
            }
        }

        return origProperty
    }
}

class EditorComponentProxy{
    static CreateProxy(component: AbstractComponent){
        let proxyHandler = new ComponentProxyHandler(component)
        let proxy = new Proxy(component, proxyHandler)
        proxyHandler.setProxy(proxy)

        return proxy
    }
}

export {EditorComponentProxy}