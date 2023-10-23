import * as React from "react"

class HierarchyItem extends React.Component<any, any> {

    render() {
        let totalChildrenCount = React.Children.count(this.props.children)
        return (
            <div className={"text-white"}>
                <div>Item</div>
                {
                    totalChildrenCount >= 1 && (
                        <div className={"flex flex-row"}>
                            < span> &emsp;|&nbsp;</span>
                            {this.props.children}
                        </div>)
                }

            </div>
        )
    }
}

class HierarchyX extends React.Component<any, any> {
    render() {
        return (<HierarchyItem>
            <HierarchyItem>
                <HierarchyItem/>
            </HierarchyItem>
        </HierarchyItem>)
    }
}

export {HierarchyX}