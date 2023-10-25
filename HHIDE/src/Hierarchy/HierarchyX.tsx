import * as React from "react"
import {v4 as uuidv4} from 'uuid';
import {sceneViewManager} from "../SceneView/SceneViewManager";
import {huahuoEngine} from "hhenginejs";
import {projectInfo} from "../SceneView/ProjectInfo";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {PropertySheet, GetObjPtr} from "hhcommoncomponents";

interface HierarchyItemProps extends React.HTMLAttributes<HTMLDivElement> {
    title: string
    hierarchyDepth?: number,
    regSetter: Function,
    targetObj?: any
}

type HierarchyItemState = {
    selected: boolean,
    isOpened: boolean
    uuid: string
}

class HierarchyItem extends React.Component<HierarchyItemProps, HierarchyItemState> {
    state: HierarchyItemState = {
        selected: false,
        isOpened: true,
        uuid: uuidv4()
    }

    setSelected(isSelected: boolean) {
        if (this.state.selected != isSelected) {
            this.state.selected = isSelected
            this.setState(this.state)
        }
    }

    componentDidMount() {
        this.props.regSetter(this.state.uuid, this.setSelected.bind(this), this.props.targetObj)
    }

    triangleClicked() {
        this.state.isOpened = !this.state.isOpened
        this.setState(this.state)
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
            tabs.push(<span key={index++}>&emsp;&emsp;</span>)
        }

        return (
            <div className="text-white select-none" style={{
                position: "relative",
                left: "0px",
                top: "0px"
            }} data-uuid={this.state.uuid} onClick={(e) => {
                this.props.onClick(e)
            }}>
                <div className={"whitespace-nowrap " + (this.state.selected ? "bg-blue-300" : "")}>
                    {tabs}
                    {
                        totalChildrenCount >= 1 && (
                            <svg style={{
                                display: "inline",
                                transform: this.state.isOpened ? null : "rotate(-90deg)"
                            }} className={"hover:scale-125 transition-all ease-in-out duration-75"}
                                 onClick={this.triangleClicked.bind(this)}
                                 xmlns="http://www.w3.org/2000/svg" width="13" height="10" viewBox="0 0 13 10">
                                <polygon points="2 1, 12 1, 7 9"></polygon>
                            </svg>
                        )
                    }
                    {' '}
                    {this.props.title || "Unknown Item"}
                </div>

                {
                    totalChildrenCount >= 1 && (
                        <div style={{
                            display: this.state.isOpened ? "block" : "none"
                        }}>
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
    objUUIDMap = new Map

    componentDidMount() {
        IDEEventBus.getInstance().on(EventNames.OBJECTSELECTED, this.onItemSelected.bind(this))

        IDEEventBus.getInstance().on("*", () => {
            this.forceUpdate()
        })
    }

    selectItemByUUID(uuid) {
        this.setters.forEach((obj) => {
            if (uuid === obj.uuid) {
                obj.setIsSelected(true)
            } else {
                obj.setIsSelected(false)
            }
        })
    }

    onItemSelected(property: PropertySheet, targetObj: any) {
        let objUUID = this.objUUIDMap.get(GetObjPtr(targetObj))
        this.selectItemByUUID(objUUID)
    }

    regSetter(uuid, setter, targetObj = null) {
        if (targetObj != null) {
            this.objUUIDMap.set(GetObjPtr(targetObj), uuid)
        }

        this.setters.push({
            uuid: uuid,
            setIsSelected: setter
        })
    }

    onItemClicked(e) {
        let uuid = e.currentTarget.dataset?.uuid
        if (uuid) {
            this.selectItemByUUID(uuid)
            e.stopPropagation()
        }
    }

    render() {
        let focusedSceneView = sceneViewManager.getFocusedSceneView()
        let store = huahuoEngine.GetStoreById(focusedSceneView.storeId)

        let items = []

        for (let layerId = 0; layerId < store.GetLayerCount(); layerId++) {
            let layer = store.GetLayer(layerId)

            let shapeItems = []
            for (let shapeIdx = 0; shapeIdx < layer.GetShapeCount(); shapeIdx++) {
                let shape = layer.GetShapeAtIndex(shapeIdx)
                let shapeItem = <HierarchyItem key={shapeIdx} title={shape.GetName()}
                                               regSetter={this.regSetter.bind(this)}
                                               onClick={this.onItemClicked.bind(this)}
                                               targetObj={shape}/>
                shapeItems.push(shapeItem)
            }

            let layerItem =
                <HierarchyItem key={layerId} title={layer.GetName()} regSetter={this.regSetter.bind(this)}
                               onClick={this.onItemClicked.bind(this)}>
                    {shapeItems}
                </HierarchyItem>

            items.push(layerItem)
        }

        return (
            <div style={{
                overflow: "auto"
            }}>
                <HierarchyItem title={projectInfo.name} hierarchyDepth={0} regSetter={this.regSetter.bind(this)}
                               onClick={this.onItemClicked.bind(this)}>
                    {items}
                </HierarchyItem>
            </div>)
    }
}

export {HierarchyX}