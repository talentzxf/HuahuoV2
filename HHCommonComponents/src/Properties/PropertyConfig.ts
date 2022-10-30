class PropertyConfig{
    elementType ?: string // Secondary type. Maybe used in some cases.
}

class FloatPropertyConfig extends PropertyConfig{
    min ?: number = 0.0 // Only number fields need this.
    max ?: number = 1.0
    step ?: number = 0.01
}

class ActionPropertyConfig extends PropertyConfig{
    action?: Function // For Button property
}

export {PropertyConfig, FloatPropertyConfig, ActionPropertyConfig}

