import * as React from "react"
import {v4 as uuidv4} from 'uuid';

interface HierarchyItemProps extends React.HTMLAttributes<HTMLDivElement> {
    title: string
    hierarchyDepth?: number,
    regSetter: Function
}

type HierarchyItemState = {
    selected: boolean,
    uuid: string
}

class HierarchyItem extends React.Component<HierarchyItemProps, HierarchyItemState> {
    state: HierarchyItemState = {
        selected: false,
        uuid: uuidv4()
    }

    setSelected(isSelected: boolean) {
        if (this.state.selected != isSelected) {
            this.state.selected = isSelected
            this.setState(this.state)
        }
    }

    componentDidMount() {
        this.props.regSetter(this.state.uuid, this.setSelected.bind(this))
    }

    render() {
        let totalChildrenCount = React.Children.count(this.props.children)

        let modifiedChildren = []
        let index = 0
        React.Children.forEach(this.props.children, (child) => {
            // @ts-ignore
            let modifiedChild = React.cloneElement(child, {
                hierarchyDepth: this.props.hierarchyDepth + 1,
                key: index++
            })

            modifiedChildren.push(modifiedChild)
        })

        index = 0
        let tabs = []
        for (let i = 0; i < this.props.hierarchyDepth; i++) {
            tabs.push(<span key={index++}>&emsp;</span>)
        }

        return (
            <div className="text-white select-none" style={{
                position: "relative",
                left: "0px",
                top: "0px"
            }} data-uuid={this.state.uuid} onClick={(e) => {
                this.props.onClick(e)
            }}>
                <div className={this.state.selected ? "bg-blue-300" : ""}>
                    {tabs}
                    {this.props.title || "Unknown Item"}
                </div>

                {
                    totalChildrenCount >= 1 && (
                        <div>
                            {modifiedChildren}
                        </div>)
                }
            </div>
        )
    }
}

type HierarchyState = {}

class HierarchyX extends React.Component<any, HierarchyState> {
    state: HierarchyState = {}

    setters = new Array

    regSetter(uuid, setter) {
        this.setters.push({
            uuid: uuid,
            setter: setter
        })
    }

    onItemClicked(e) {
        let uuid = e.currentTarget.dataset?.uuid
        if (uuid) {
            this.setters.forEach((obj) => {
                if (uuid === obj.uuid) {
                    obj.setter(true)
                } else {
                    obj.setter(false)
                }
            })
            e.stopPropagation()
        }
    }

    render() {
        return (
            <div style={{
                overflow: "hidden"
            }}>
                <HierarchyItem title="Root" hierarchyDepth={0} regSetter={this.regSetter.bind(this)}
                               onClick={this.onItemClicked.bind(this)}>
                    <HierarchyItem title="layer1" regSetter={this.regSetter.bind(this)}
                                   onClick={this.onItemClicked.bind(this)}>
                        <HierarchyItem title="shape1" regSetter={this.regSetter.bind(this)}
                                       onClick={this.onItemClicked.bind(this)}/>
                        <HierarchyItem title="shape2" regSetter={this.regSetter.bind(this)}
                                       onClick={this.onItemClicked.bind(this)}/>
                    </HierarchyItem>
                    <HierarchyItem title="layer2" regSetter={this.regSetter.bind(this)}
                                   onClick={this.onItemClicked.bind(this)}>
                        <HierarchyItem title="shape21" regSetter={this.regSetter.bind(this)}
                                       onClick={this.onItemClicked.bind(this)}/>
                        <HierarchyItem title="shape22" regSetter={this.regSetter.bind(this)}
                                       onClick={this.onItemClicked.bind(this)}/>
                    </HierarchyItem>
                    <HierarchyItem title="layer3" regSetter={this.regSetter.bind(this)}
                                   onClick={this.onItemClicked.bind(this)}>
                        <HierarchyItem title="shape31" regSetter={this.regSetter.bind(this)}
                                       onClick={this.onItemClicked.bind(this)}/>
                        <HierarchyItem title="shape32" regSetter={this.regSetter.bind(this)}
                                       onClick={this.onItemClicked.bind(this)}/>
                    </HierarchyItem>
                </HierarchyItem>
            </div>)
    }
}

export {HierarchyX}