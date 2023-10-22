import * as React from "react"
import {GetPropertyReactGenerator, PropertyEntry, PropertyProps} from "./BasePropertyX";

const SELECT_TAB_CLASS = "inline-block p-4 text-blue-600 bg-gray-100 rounded-t-lg active dark:bg-gray-800 dark:text-blue-500"
const UNSELECT_TAB_CLASS = "inline-block p-4 rounded-t-lg hover:text-gray-600 hover:bg-gray-50 dark:hover:bg-gray-800 dark:hover:text-gray-300"

type GroupPropertyState = {
    selectedIndex: number
}

class GroupPropertyX extends React.Component<PropertyProps, any> {
    state: GroupPropertyState = {
        selectedIndex: 0
    }

    render() {
        let property = this.props.property

        let titleDivs = []

        let contentDivs = []

        let index = 0;

        for (let childProperty of property.config.children) {
            if (childProperty.hide)
                continue

            let childTitleDiv = <div key={index} data-tab-index={index} className={
                index == this.state.selectedIndex ? SELECT_TAB_CLASS : UNSELECT_TAB_CLASS
            } onClick={
                (evt) => {
                    let tabIndx = (evt.target as HTMLDivElement).dataset.tabIndex
                    this.state.selectedIndex = Number(tabIndx)
                    this.setState(this.state)
                }
            }>{i18n.t(childProperty.key)}</div>


            let contentGenerator = GetPropertyReactGenerator(childProperty.type)
            if (contentGenerator) {
                let contentProps = {
                    key: index,
                    property: childProperty
                }

                let divStyle = {
                    display: "block"
                }

                if (contentProps.key != this.state.selectedIndex) {
                    divStyle.display = "none"
                }

                let reactElement = React.createElement(contentGenerator, contentProps)
                let reactElementDiv = React.createElement("div", {
                    key: index,
                    style: divStyle
                }, reactElement)

                contentDivs.push(reactElementDiv)
            }

            titleDivs.push(childTitleDiv)
            index++
        }

        return (
            <PropertyEntry property={property}>
                <div className={"flex flex-col"}>
                    <div>
                        {titleDivs}
                    </div>
                    <div>
                        {contentDivs}
                    </div>
                </div>
            </PropertyEntry>
        )
    }
}

export {GroupPropertyX}