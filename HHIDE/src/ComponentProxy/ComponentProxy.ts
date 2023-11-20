import {AbstractComponent, huahuoEngine, PropertyCategory, PropertyDef} from "hhenginejs";
import {CustomFieldConfig, getMethodsAndVariables, GetObjPtr, PropertyType} from "hhcommoncomponents";
import {propertySheetFactory} from "./PropertySheetBuilderFactory";

// Key is: className#fieldName
// Value is the React.Component of this field
let customFieldContentXMap: Map<string, React.Component> = new Map()

function registerCustomFieldPropertyX(className: string, fieldName: string, reactComponent) {
    let fieldFulleName = className + "#" + fieldName
    customFieldContentXMap.set(fieldFulleName, reactComponent)
}

function getCustomFieldXComponent(className: string, fieldName: string): React.Component {
    let fieldFullName = className + "#" + fieldName
    return customFieldContentXMap.get(fieldFullName)
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

    proxyFunctionMap: Map<Function, Function> = new Map()

    get(target, propKey, receiver) {
        const origProperty = target[propKey]

        let _this = this

        if (origProperty instanceof Function) {
            let proxiedFunction = this.proxyFunctionMap.get(origProperty)
            if (proxiedFunction == null) {
                proxiedFunction = function (...args) {
                    if (!_this.functionMap.has(origProperty.name)) {
                        return origProperty.apply(this, args)
                    }

                    return _this.functionMap.get(origProperty.name).apply(this, args)
                }

                this.proxyFunctionMap.set(origProperty, proxiedFunction)
            }

            return proxiedFunction
        }

        if (this[propKey] && this[propKey] instanceof Function) {
            return this[propKey].bind(this)
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
            (entry, index) => {
                if (entry.hasOwnProperty("rawObjPtr") && entry["rawObjPtr"] == GetObjPtr(thisComponent)) {
                    properties[index] = thisComponent.getPropertySheet()
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
            if (myPropertySheet){
                propertySheet.addProperty(myPropertySheet)
            }
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
                    thisComponent.enableComponent(true)
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
                    if (propertyMeta.config == null || propertyMeta.config["contentGenerator"] == null) {

                        propertyMeta = {...propertyMeta} // Clone it to avoid affecting other objects. Shallow copy should be enough.

                        let constructorName = Object.getPrototypeOf(thisComponent).constructor.name

                        let reactComponent = getCustomFieldXComponent(constructorName, propertyMeta.key)

                        propertyMeta.config = {
                            fieldName: propertyMeta["key"],
                            targetComponent: this.proxy,
                            contentGenerator: reactComponent
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

        componentConfigSheet["rawObjPtr"] = thisComponent.rawObj.ptr

        return componentConfigSheet
    }
}

class EditorComponentProxy {
    static CreateProxy(component: AbstractComponent) {
        if (component.getProxy != null) { // Already proxied.
            return component
        }

        let proxyHandler = new ComponentProxyHandler(component)
        let proxy = new Proxy(component, proxyHandler)
        proxyHandler.setProxy(proxy)

        return proxy
    }
}

export {EditorComponentProxy, registerCustomFieldPropertyX}