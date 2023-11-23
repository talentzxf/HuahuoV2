import * as React from "react";
import {GetPropertyReactGenerator, PropertyEntry, PropertyProps} from "./BasePropertyX";
import {i18n} from "hhcommoncomponents";

// Implement Accordion: https://css-tricks.com/using-css-transitions-auto-dimensions/

type ComponentPropertyState = {
    isOpen: boolean
    contentHeight: number
    isTransitioning: boolean
}

class ComponentPropertyX extends React.Component<PropertyProps, ComponentPropertyState> {
    state: ComponentPropertyState = {
        isOpen: true,
        contentHeight: 10000,
        isTransitioning: false
    }

    contentRef

    constructor(props) {
        super(props);
        this.contentRef = React.createRef()
    }

    contentClass: string = "grid grid-cols-2 duration-500 ease-in-out transition-[max-height] overflow-hidden"

    openSection() {
        this.state.isOpen = true
        this.state.contentHeight = this.contentRef.current.scrollHeight
        this.state.isTransitioning = true

        this.setState(this.state)
    }

    closeSection() {
        this.state.isOpen = false
        this.state.contentHeight = 0
        this.state.isTransitioning = true

        this.setState(this.state)
    }

    setContentHeight() {
        if(this.contentRef.current == null) // Not sure why this happens?
            return

        if (!this.state.isTransitioning) {
            if (this.state.isOpen && this.state.contentHeight != this.contentRef.current.scrollHeight) {

                if(this.contentRef.current.scrollHeight > 0){ // If the scrollHeight is 0, maybe just means we switched to another tab.
                    this.state.contentHeight = this.contentRef.current.scrollHeight
                    this.setState(this.state)
                }
            }
        }
    }

    componentDidMount() {
        this.setContentHeight()
    }

    componentDidUpdate(prevProps: Readonly<PropertyProps>, prevState: Readonly<ComponentPropertyState>, snapshot?: any) {
        this.setContentHeight()
    }

    onClick(e) {
        this.state.isOpen = !this.state.isOpen

        if (this.state.isOpen) {
            this.openSection()
        } else {
            this.closeSection()
        }
    }

    onTransitionEnd(e) {
        this.state.isTransitioning = false
        this.setState(this.state)
    }

    render() {
        let property = this.props.property

        if (!property.config || !property.config.children || property.config.children.length == 0) {
            return null
        }

        let index = 0

        let childElements = []

        for (let childProperty of property.config.children) {
            if (childProperty.hide)
                continue
            // In child property, setter is actually inserter if not explicitly set.
            if (childProperty.inserter && !childProperty.hasOwnProperty("setter")) {
                childProperty.setter = childProperty.inserter
            }

            if (childProperty.registerValueChangeFunc) {
                childProperty.registerValueChangeFunc(
                    () => {
                        requestAnimationFrame(this.setContentHeight.bind(this))
                    }
                ) // The height maybe changed for each component, need recalculated in next frame.
            }

            let generator = GetPropertyReactGenerator(childProperty.type)
            if (generator) {
                let childReactElement = React.createElement(generator, {
                    key: index++,
                    property: childProperty
                })
                childElements.push(childReactElement)
            } else {
                console.warn("Unknown generator:" + childProperty.key + " type index:" + childProperty.type)
            }
        }

        let propertyKey = i18n.t(this.props.property.key)

        return (
            <PropertyEntry
                className="flex flex-col w-full first:rounded-t-lg last:rounded-b-lg border border-neutral-200 bg-white"
                property={this.props.property}
                noDefaultTitle={true}>

                <button className="hover:bg-blue-300 bg-blue-100 w-full focus:ring-4 focus:ring-cyan-300 focus:border-cyan-200 text-left
            focus:outline-none" onClick={this.onClick.bind(this)}>
                    {propertyKey}
                </button>

                {
                    (
                        <div ref={this.contentRef} className={this.contentClass}
                             style={{"maxHeight": this.state.contentHeight}}
                             onTransitionEnd={this.onTransitionEnd.bind(this)}>
                            {childElements}
                        </div>
                    )
                }

            </PropertyEntry>
        )
    }
}

export {ComponentPropertyX}