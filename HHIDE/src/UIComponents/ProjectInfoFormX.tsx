import * as React from "react"
import {CloseBtn} from "./CloseBtn";
import {CSSUtils} from "../Utilities/CSSUtils";

class ProjectInfoFormX extends React.Component<any, any> {
    render() {
        return (
            <div className="select-none flex flex-col items-center justify-center mx-auto">
                <div
                    className="bg-white rounded-lg drop-shadow-2xl dark:border md:mt-0 sm:max-w-md xl:p-0
                        dark:bg-gray-800 dark:border-gray-700">
                    <form className="p-4 space-y-4 divide-y divide-gray-300" action="#">
                        <div className="flex align-middle">
                            <h5 className="px-2 text-xl font-bold leading-tight tracking-tight text-gray-900 md:text-2xl dark:text-white">
                                {i18n.t("ProjectInfo")}
                            </h5>
                            <CloseBtn onclick={this.props?.closeForm}></CloseBtn>
                        </div>
                        <div className="w-full flex flex-col">
                            <label className="font-bold">{i18n.t("ProjectName")}</label>
                            <div className="flex items-center">
                                <input className={CSSUtils.getInputStyle()} placeholder="Enter Project Name"/>
                                <img className="w-[20px] h-[20px]"/>
                            </div>
                            <label className="font-bold">{i18n.t("ProjectDescription")}</label>
                            <textarea placeholder="Enter Project Description"
                                      className="block p-2.5 w-full text-sm text-gray-900 bg-gray-50 rounded-lg border border-gray-300 focus:ring-blue-500 focus:border-blue-500 dark:bg-gray-700 dark:border-gray-600 dark:placeholder-gray-400 dark:text-white dark:focus:ring-blue-500 dark:focus:border-blue-500"></textarea>
                            <label className="font-bold">{i18n.t("ProjectPreview")}</label>
                            <div className="w-[300px] h-auto">
                                <canvas className="border-cyan-200 border-2"></canvas>
                            </div>
                        </div>
                        <div className="w-full">
                            <button className={CSSUtils.getButtonClass("primary")}>OK</button>
                        </div>
                    </form>
                </div>
            </div>
        )
    }
}

export {ProjectInfoFormX}