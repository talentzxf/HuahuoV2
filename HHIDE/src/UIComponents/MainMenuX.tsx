import * as React from "react"
import {ToolBarX} from "./ToolBarX";
import {EditorToolBarX} from "./EditorToolBarX";
import {PlayerControllerX} from "../AnimationPlayer/PlayerControllerX";
import {UserInfoBarX} from "../Identity/UserInfoBarX";

function getButtonClz() {
    return "bg-transparent hover:bg-blue-500 text-blue-700 font-semibold " +
        "hover:text-white border border-blue-500 hover:border-transparent w-10 h-10 first:rounded-l-lg last:rounded-r-lg "
}

function getImgClz() {
    return "hover:p-0 p-1 transition-all ease-in-out"
}

function imgButton(icon, title: string, onClick = null, style = null) {
    return (
        <button className={getButtonClz()} onClick={onClick}>
            <img className={getImgClz()} title={title}
                 src={`data:image/svg+xml;utf8,${encodeURIComponent(icon)}`} style={style}/></button>
    )
}

class MainMenuX extends React.Component<any, any> {
    render() {
        return (
            <div className="flex flex-row select-none text-white">
                <div className="w-[20%]">
                    <ToolBarX></ToolBarX>
                </div>
                <div className="w-[20%]">
                    <EditorToolBarX></EditorToolBarX>
                </div>
                <div className="w-[30%]">
                    <PlayerControllerX></PlayerControllerX>
                </div>
                <div className="w-full">
                    <UserInfoBarX></UserInfoBarX>
                </div>

            </div>
        )
    }
}

export {MainMenuX, imgButton}