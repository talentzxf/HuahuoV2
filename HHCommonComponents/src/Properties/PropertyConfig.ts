import {Property} from "./PropertySheet";

class PropertyConfig {
    elementType ?: string // Secondary type. Maybe used in some cases.
}

interface CustomFieldContentDivGenerator {
    refresh()
    generateDiv(property: Property): HTMLDivElement
}

class CustomFieldConfig extends PropertyConfig {
    fieldName: string
    contentDivGenerator?: Function
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

class ComponentProperty extends PropertyConfig {
    children: []
    enabler: Function
    disabler: Function
    isActive: Function
}

export {PropertyConfig, FloatPropertyConfig, ActionPropertyConfig, SubComponentArrayProperty, CustomFieldConfig, CustomFieldContentDivGenerator, ComponentProperty}


