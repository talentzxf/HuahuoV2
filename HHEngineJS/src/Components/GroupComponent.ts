import {AbstractComponent, Component} from "./AbstractComponent";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";
import {clzObjectFactory} from "../CppClassObjectFactory";

/**
 * GroupComponent is a special Component. It's responsibility is to store other components.
 */
@Component({compatibleShapes: []})
class GroupComponent extends AbstractComponent {
    private _subComponents: Array<AbstractComponent> // This array can't be inited in the constructor.

    constructor(rawObj?, isMirage = false) {
        super(rawObj, isMirage);

        if(rawObj){
            let subComponentCount = this.rawObj.GetSubComponentCount()
            for(let subComponentIdx = 0 ; subComponentIdx < subComponentCount; subComponentIdx++){
                let subComponentRawObj = this.rawObj.GetSubComponentByIdx(subComponentIdx)

                if(this.getComponentByRawObj(subComponentRawObj) == null){
                    let subComponentConstructor = clzObjectFactory.GetClassConstructor(subComponentRawObj.GetTypeName())
                    let subComponent = new subComponentConstructor(subComponentRawObj)
                    this.addSubComponent(subComponent)
                }
            }
        }
    }

    get subComponents(): Array<AbstractComponent>{
        if(this._subComponents == null){
            this._subComponents = new Array<AbstractComponent>()
        }
        return this._subComponents
    }

    setBaseShape(baseShape: BaseShapeJS) {
        super.setBaseShape(baseShape);
        for(let subComponent of this.subComponents){
            subComponent.setBaseShape(baseShape) // Set baseshape for all it's children.
        }
    }

    addSubComponent(component: AbstractComponent) {
        this.rawObj.AddSubComponent(component.rawObj) // Add subcomponent in cpp side.

        this.subComponents.push(component)

        component.setBaseShape(this.baseShape)
    }

    getComponentByRawObj(componentRawObj) {
        return this.subComponents.find((component) => {
            return component.rawObj.ptr == componentRawObj.ptr
        })
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        for (let subComponent of this.subComponents) {
            if(subComponent.isComponentActive())
                subComponent.afterUpdate(force)
        }
    }
}

export {GroupComponent}