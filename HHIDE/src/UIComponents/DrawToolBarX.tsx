import * as React from "react"
import {shapes} from "../ShapeDrawers/Shapes";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";
import {BaseShapeDrawer} from "../ShapeDrawers/BaseShapeDrawer";
import {DrawToolBar} from "./DrawToolBar";

type DrawToolBarXState = {
    currentActiveDrawer: BaseShapeDrawer
}

class DrawToolBarX extends React.Component<any, DrawToolBarXState> {
    state: DrawToolBarXState = {
        currentActiveDrawer: null
    }

    onClick(e) {
        let shapeDrawName = e.currentTarget.dataset.buttonShapeName
        let shapeDrawer = this.shapeDrawerNameShapeMap.get(shapeDrawName)

        IDEEventBus.getInstance().emit(EventNames.DRAWSHAPEBEGINS, shapeDrawer)

        if (this.state.currentActiveDrawer == shapeDrawer)
            return;

        this.state.currentActiveDrawer = shapeDrawer
        this.state.currentActiveDrawer.isSelected = true

        this.setState(this.state)
    }

    shapeDrawerNameShapeMap = new Map

    render() {
        let toolBars = []
        for (let shapeDrawer of shapes) {
            let imgSrc = null
            let imgClass = null
            if (shapeDrawer.imgClass != "unknown_img") {
                imgClass = shapeDrawer.imgClass
            } else {
                imgSrc = shapeDrawer.imgCss
            }

            this.shapeDrawerNameShapeMap.set(shapeDrawer.name, shapeDrawer)

            toolBars.push(
                (
                    <button style={{
                        backgroundColor: (this.state.currentActiveDrawer == shapeDrawer) ? "#42b983" : "white"
                    }} onClick={this.onClick.bind(this)} data-button-shape-name={shapeDrawer.name}>
                        <img src={imgSrc}
                             className={imgClass + " m-2 w-6 hover:m-1 hover:w-8 h-6 hover:h-8 transition-all ease-in-out"}
                             title={i18n.t(shapeDrawer.name)}/>
                    </button>
                ))
        }

        return (<div>
            {
                toolBars
            }
        </div>)
    }
}

export {DrawToolBarX}