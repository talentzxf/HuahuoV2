import * as React from "react"
import {PropertyEntry, PropertyProps} from "./BasePropertyX";

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

        let index = 0;

        for (let childProperty of property.config.children) {
            if (childProperty.hide)
                continue

            let childTitleDiv = <div data-tab-index={index} className={
                index == this.state.selectedIndex ? SELECT_TAB_CLASS : UNSELECT_TAB_CLASS
            }>{i18n.t(childProperty.key)}</div>

            titleDivs.push(childTitleDiv)
            index++
        }

        return (
            <PropertyEntry property={property}>
                {titleDivs}
            </PropertyEntry>
        )
    }
}

export {GroupPropertyX}