import * as React from "react"
import {shapes} from "../ShapeDrawers/Shapes";

class DrawToolBarX extends React.Component<any, any> {
    render() {
        let toolBars = []
        for (let shape of shapes) {
            let imgSrc = null
            let imgClass = null
            if (shape.imgClass != "unknown_img") {
                imgClass = shape.imgClass
            } else {
                imgSrc = shape.imgCss
            }

            toolBars.push((
                <button>
                    <img src={imgSrc} className={imgClass} title={i18n.t(shape.name)} style={{width: 20, height: 20}}
                         onClick={shape.onClicked.bind(shape)}/>
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