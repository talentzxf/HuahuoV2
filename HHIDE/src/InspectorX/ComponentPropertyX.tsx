import * as React from "react";
import {GetPropertyReactGenerator, PropertyEntry, PropertyProps} from "./BasePropertyX";
import {i18n} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";

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
        if (this.contentRef.current == null) // Not sure why this happens?
            return

        if (!this.state.isTransitioning) {
            if (this.state.isOpen && this.state.contentHeight != this.contentRef.current.scrollHeight) {

                if (this.contentRef.current.scrollHeight > 0) { // If the scrollHeight is 0, maybe just means we switched to another tab.
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
                noDefaultTitle={true}
            >

                <div className="flex flex-row hover:bg-blue-300 bg-blue-100 w-full focus:ring-4 focus:ring-cyan-300 focus:border-cyan-200 text-left
            focus:outline-none cursor-pointer select-none align-baseline" onClick={this.onClick.bind(this)}>
                    <div className="w-full mx-1 flex-1"> {propertyKey} </div>
                    {
                        this.props.property?.config?.enabler && this.props.property?.config?.isActive &&
                        <div className={CSSUtils.getButtonClass("teal") + " rounded mx-1 px-1"} onClick={(e) => {
                            if (this.props.property.config.isActive()) {
                                this.props.property.config.disabler()
                            } else {
                                this.props.property.config.enabler()
                            }
                            e.stopPropagation()
                            e.preventDefault()
                            this.forceUpdate()
                        }}>{
                            this.props.property?.config?.isActive() ? i18n.t("inspector.disable") : i18n.t("inspector.enable")
                        }</div>
                    }
                </div>

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