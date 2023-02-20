import {AbstractComponent, Component} from "./AbstractComponent";

/**
 * GroupComponent is a special Component. It's responsibility is to store other components.
 */
@Component({compatibleShapes: []})
class GroupComponent extends AbstractComponent {
    subComponents: Array<AbstractComponent>

    addSubComponent(component: AbstractComponent) {
        if (this.subComponents == null) {
            this.subComponents = new Array<AbstractComponent>()
        }
        this.subComponents.push(component)
    }

    getComponentByRawObj(componentRawObj) {
        if (this.subComponents == null)
            return null

        return this.subComponents.find((component) => {
            return component.rawObj == componentRawObj
        })
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        if (this.subComponents == null)
            return

        for (let subComponent of this.subComponents) {
            subComponent.afterUpdate(force)
        }
    }
}

export {GroupComponent}