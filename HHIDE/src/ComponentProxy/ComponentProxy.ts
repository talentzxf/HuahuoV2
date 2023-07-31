import {AbstractComponent} from "hhenginejs";
import {getMethodsAndVariables} from "hhcommoncomponents";
import {huahuoEngine} from "hhenginejs";
import {PropertyCategory, PropertyDef} from "hhenginejs";
import {PropertyType} from "hhcommoncomponents";
import {CustomFieldConfig} from "hhcommoncomponents";
import {propertySheetFactory} from "./PropertySheetBuilderFactory";

// Key is: className#fieldName
// Value is the constructor of the divContent generator
let customFieldContentDivGeneratorMap: Map<string, Function> = new Map()

function registerCustomFieldContentDivGeneratorConstructor(className: string, fieldName: string, constructor: Function) {
    let fieldFullName = className + "#" + fieldName
    customFieldContentDivGeneratorMap.set(fieldFullName, constructor)
}

function getCustomFieldContentDivGeneratorConstructor(className: string, fieldName: string): Function {
    let fieldFullName = className + "#" + fieldName
    return customFieldContentDivGeneratorMap.get(fieldFullName)
}

class ComponentProxyHandler {
    targetComponent: AbstractComponent

    functionMap: Map<string, Function> = new Map()

    proxy

    setProxy(proxy) {
        this.proxy = proxy
    }

    constructor(targetComponent) {
        this.targetComponent = targetComponent

        let _this = this
        getMethodsAndVariables(this, true).forEach((key) => {
            if (typeof _this[key] == "function" &&
                key != "constructor" &&
                !key.startsWith("__") &&
                key != "get") {
                _this.functionMap.set(key, _this[key].bind(_this))
            }
        })
    }

    getPrototypeOf(target) {
        return Object.getPrototypeOf(this.targetComponent);
    }

    get(target, propKey, receiver) {
        const origProperty = target[propKey]

        let _this = this

        if (origProperty instanceof Function) {
            return function (...args) {
                if (!_this.functionMap.has(origProperty.name)) {
                    return origProperty.apply(this, args)
                }

                return _this.functionMap.get(origProperty.name).apply(this, args)
            }
        }

        if (this[propKey] && this[propKey] instanceof Function) {
            return this[propKey]
        }

        return origProperty
    }

    getProxy(obj) {
        if (obj instanceof ComponentProxyHandler)
            return obj.proxy

        return obj
    }

    updateComponentPropertySheet(propertySheet) {
        let thisComponent: AbstractComponent = this.getProxy(this)
        let properties = propertySheet.getProperties()
        properties.forEach(
            (entry) => {
                if (entry.hasOwnProperty("rawObjPtr") && entry["rawObjPtr"] == thisComponent.rawObj.ptr) {
                    entry = thisComponent.getPropertySheet()
                }
            })

        propertySheet.setProperties(properties)
    }

    propertySheetInited = false

    initPropertySheet(propertySheet) {
        if (!this.propertySheetInited) {
            this.propertySheetInited = true

            let thisComponent: AbstractComponent = this.getProxy(this)

            let myPropertySheet = this.getPropertySheet()
            myPropertySheet["rawObjPtr"] = thisComponent.rawObj.ptr
            if (myPropertySheet)
                propertySheet.addProperty(myPropertySheet)
        }
    }

    detachFromCurrentShape() {
        this.propertySheetInited = false // This shape has been removed from the shape, ret property sheet property.

        this.targetComponent.detachFromCurrentShape.apply(this.proxy)
    }

    getPropertySheet() {
        let thisComponent: AbstractComponent = this.getProxy(this)

        let componentConfigSheet = {
            key: thisComponent.getTypeName(),
            type: PropertyType.COMPONENT,
            targetObject: thisComponent.baseShape,
            config: {
                children: [],
                enabler: () => {
                    thisComponent.enableComponent()
                },
                disabler: () => {
                    thisComponent.disableComponent()
                },
                isActive: () => {
                    return thisComponent.isComponentActive()
                }
            }
        }

        if (!thisComponent.isBuiltIn) { //BuiltIn components can't be deleted.
            componentConfigSheet.config["deleter"] = () => {
                huahuoEngine.dispatchEvent("HHIDE", "DeleteComponent", thisComponent)
            }
        }

        const properties: PropertyDef[] = Reflect.getMetadata(AbstractComponent.getMetaDataKey(), thisComponent)
        if (properties != null) {
            for (let propertyMeta of properties) {
                if (propertyMeta.type == PropertyCategory.customField) {
                    if (propertyMeta.config == null || propertyMeta.config["contentDivGenerator"] == null) {

                        propertyMeta = {...propertyMeta} // Clone it to avoid affecting other objects. Shallow copy should be enough.

                        let constructorName = Object.getPrototypeOf(thisComponent).constructor.name

                        let divGeneratorConstructor = getCustomFieldContentDivGeneratorConstructor(constructorName, propertyMeta.key)

                        // @ts-ignore
                        let contentDivGenerator = new divGeneratorConstructor(thisComponent)
                        propertyMeta.config = {
                            fieldName: propertyMeta["key"],
                            contentDivGenerator: contentDivGenerator
                        } as CustomFieldConfig
                    }
                }
                let propertySheetEntry = propertySheetFactory.createEntry(thisComponent, propertyMeta, thisComponent.valueChangeHandler)
                if (propertySheetEntry != null) {
                    componentConfigSheet.config.children.push(propertySheetEntry)
                }
            }
        }

        let keyFramePropertySheet = propertySheetFactory.createEntryByNameAndCategory("keyframes", PropertyCategory.keyframeArray)

        keyFramePropertySheet["getter"] = thisComponent.getKeyFrames.bind(thisComponent)
        keyFramePropertySheet["targetObject"] = thisComponent.baseShape
        keyFramePropertySheet["deleter"] = thisComponent.baseShape.deleteComponentKeyFrame(thisComponent.getTypeName()).bind(thisComponent.baseShape)

        componentConfigSheet.config.children.push(keyFramePropertySheet)

        return componentConfigSheet
    }
}

class EditorComponentProxy {
    static CreateProxy(component: AbstractComponent) {
        let proxyHandler = new ComponentProxyHandler(component)
        let proxy = new Proxy(component, proxyHandler)
        proxyHandler.setProxy(proxy)

        return proxy
    }
}

export {EditorComponentProxy, registerCustomFieldContentDivGeneratorConstructor}