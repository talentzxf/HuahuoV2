import {AbstractComponent, Component} from "./AbstractComponent";

/**
 * ComponentGroup is a special Component. It's responsibility is to store other components.
 */
@Component()
class GroupComponent extends AbstractComponent{
    subComponents: Array<AbstractComponent> = new Array<AbstractComponent>()

    addSubComponent(component: AbstractComponent){
        this.subComponents.push(component)
    }

    getComponentByRawObj(componentRawObj){
        return this.subComponents.find((component)=>{
            return component.rawObj == componentRawObj
        })
    }

    afterUpdate(force: boolean = false) {
        super.afterUpdate(force);

        for(let subComponent of this.subComponents){
            subComponent.afterUpdate(force)
        }
    }
}

export {GroupComponent}