import {PropertyEntry, GetPropertyReactGenerator} from "./BasePropertyDivGeneratorX";
import * as React from "react"

class PanelPropertyReactGeneratorX extends React.Component<any, any>{

}

class ComponentPropertyGeneratorX extends React.Component<any, any>{
    render(){
        let property = this.props.property

        if(!property.config || !property.config.children || property.config.children.length == 0){
            return null
        }

        let childElements = []

        for(let childProperty of property.config.children){
            if(childProperty.hide)
                continue
            // In child property, setter is actually inserter if not explicitly set.
            if(childProperty.inserter && !childProperty.hasOwnProperty("setter")){
                childProperty.setter = childProperty.inserter
            }

            let generator = GetPropertyReactGenerator(childProperty.type)
            if(generator){
                let childReactElement = React.createElement(generator, {
                    key: Math.random(),
                    property: childProperty
                })
                childElements.push(childReactElement)
            }
        }

        let propertyKey = i18n.t(this.props.property.key)

        return (
            <PropertyEntry key={Math.random()} divStyle="flex flex-col w-full first:rounded-t-lg last:rounded-b-lg border border-neutral-200 bg-white"
                property={this.props.property}>

                <button className="hover:bg-blue-300 bg-blue-100 w-full focus:ring-4 focus:ring-cyan-300 focus:border-cyan-200 text-left
            focus:outline-none">
                    {propertyKey}
                </button>

                {childElements}
            </PropertyEntry>
        )
    }
}

export {PanelPropertyReactGeneratorX, ComponentPropertyGeneratorX}