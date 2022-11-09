class CppClassObjectFactory{
    clzNameConstructorMap: Map<string, Function> = new Map<string, Function>();

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
}

let clzObjectFactory = window["clzObjectFactory"]
if (!clzObjectFactory) {
    clzObjectFactory = new CppClassObjectFactory()
    window["clzObjectFactory"] = clzObjectFactory
}
export {clzObjectFactory}