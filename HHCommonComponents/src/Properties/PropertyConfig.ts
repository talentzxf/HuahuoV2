import {Property} from "./PropertySheet";

class PropertyConfig {
    elementType ?: string // Secondary type. Maybe used in some cases.
}

interface CustomFieldContentDivGenerator {
    refresh()

    generateDiv(property: Property): HTMLDivElement
}

interface CustomFieldContentXGenerator {
    generateReactNode(property: Property)
}

class CustomFieldConfig extends PropertyConfig {
    fieldName: string
    targetComponent?: any
    contentGenerator?: any
}

class FloatPropertyConfig extends PropertyConfig {
    min ?: number = 0.0 // Only number fields need this.
    max ?: number = 1.0
    step ?: number = 0.01
    getKeyFrameCurve?: Function = null
}

class Vector2PropertyConfig extends PropertyConfig {
    getKeyFrameCurves?: Function[] = null
}

class ActionPropertyConfig extends PropertyConfig {
    action?: Function // For Button property
}

class SubComponentArrayProperty extends PropertyConfig {
    subComponentTypeName: string;
}

class ShapeArrayProperty extends PropertyConfig {
    allowDuplication: boolean = false;
}

class ComponentProperty extends PropertyConfig {
    children: []
    enabler: Function
    disabler: Function
    isActive: Function
}

class GetterProperty extends PropertyConfig {
    observedFields: []
}

class StringProperty extends PropertyConfig {
    options: []
    maxLength: -1
}

export {
    PropertyConfig, FloatPropertyConfig, ActionPropertyConfig,GetterProperty,
    SubComponentArrayProperty, CustomFieldConfig, CustomFieldContentDivGenerator,
    ComponentProperty, ShapeArrayProperty, StringProperty, CustomFieldContentXGenerator
}


