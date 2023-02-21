import {AbstractComponent, Component} from "./AbstractComponent";
import {BaseShapeJS} from "../Shapes/BaseShapeJS";

/**
 * GroupComponent is a special Component. It's responsibility is to store other components.
 */
@Component({compatibleShapes: []})
class GroupComponent extends AbstractComponent {
    private _subComponents: Array<AbstractComponent> // This array can't be inited in the constructor.

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
            return component.rawObj == componentRawObj
        })
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        for (let subComponent of this.subComponents) {
            subComponent.afterUpdate(force)
        }
    }
}

export {GroupComponent}