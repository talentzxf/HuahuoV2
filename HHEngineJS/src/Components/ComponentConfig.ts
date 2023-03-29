class ComponentConfig{
    compatibleShapes ?= ["BaseShapeJS"]
    maxCount ?= 1
    cppClassName ?= "CustomComponent" // The corresponding class name in Cpp side.
}

export {ComponentConfig}