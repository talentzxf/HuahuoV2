import {ComponentConfig} from "./Components/ComponentConfig";

class CppClassObjectFactory{
    clzNameConstructorMap: Map<string, Function> = new Map<string, Function>();

    componentNameComponentPropertyMap: Map<string, ComponentConfig> = new Map<string, ComponentConfig>()
    RegisterClass(clzName: string, constructor: Function) {
        this.clzNameConstructorMap.set(clzName, constructor)
    }

    GetClassConstructor(clzName: string) {
        return this.clzNameConstructorMap.get(clzName)
    }

    getAllCompatibleComponents(targetObj){
        this.clzNameConstructorMap.forEach((clzName, constructor)=>{

        })
    }

    RegisterComponent(componentName, config){
        this.componentNameComponentPropertyMap.set(componentName, config)
    }
}

let clzObjectFactory = window["clzObjectFactory"]
if (!clzObjectFactory) {
    clzObjectFactory = new CppClassObjectFactory()
    window["clzObjectFactory"] = clzObjectFactory
}
export {clzObjectFactory}