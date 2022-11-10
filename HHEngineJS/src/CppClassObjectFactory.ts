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

    isInheritedFromClzName(obj, clzName): boolean{
        let curProto = obj
        while(curProto != null){
            if(curProto.constructor.name == clzName)
                return true

            curProto = curProto.prototype
        }

        return false;
    }

    getAllCompatibleComponents(targetObj){
        let returnComponentNames = []
        this.componentNameComponentPropertyMap.forEach((componentConfig, componentTypeName)=>{
            let isCompatible = true
            if(componentConfig){
                let isCompatibleWithShape = false
                if(componentConfig.compatibleShapes && componentConfig.compatibleShapes.length > 0){
                    for(let shapeName of componentConfig.compatibleShapes){
                        if(this.isInheritedFromClzName(targetObj, shapeName)){
                            isCompatibleWithShape = true
                            break;
                        }
                    }
                }

                let matchesComponentCount = false
                if(isCompatibleWithShape){
                    let currentComponentCount = targetObj.getComponentCountByTypeName(componentTypeName)
                    if(currentComponentCount + 1 < componentConfig.maxCount)
                        matchesComponentCount = true
                }

                isCompatible = isCompatibleWithShape && matchesComponentCount
            }

            if(isCompatible){
                returnComponentNames.push(componentTypeName)
            }
        })

        return returnComponentNames
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