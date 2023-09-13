import * as React from "react"
import {CloseBtn} from "./CloseBtn";
import {FormProps} from "../Utilities/FormManager";
import {api} from "../RESTApis/RestApi"
import {projectManager} from "../HuaHuoEngine/ProjectManager";
import {projectInfo} from "../SceneView/ProjectInfo";
import {huahuoEngine} from "hhenginejs";
import {HHToast} from "hhcommoncomponents";

type ProjectListProps = FormProps & {
    title: string
    pageSize: number
}

type BinaryFile = {
    name: string
    description: string
    id: number
    createdBy: string
}

type ProjectListState = {
    loaded: boolean
    totalPage: number
    currentPage: number
    binaryFiles?: BinaryFile[]
}

class ProjectListFormX extends React.Component<ProjectListProps, ProjectListState> {
    state: ProjectListState = {
        loaded: false,
        totalPage: -1,
        currentPage: 1,
        binaryFiles: []
    }

    projectInfoMap: Map<number, BinaryFile> = new Map

    refreshPage() {
        api.listProjects((result) => {
            this.state.loaded = true
            this.state.totalPage = result.totalCount / this.props.pageSize
            this.state.binaryFiles = result.binaryFiles

            this.setState(this.state)

        }, this.state.currentPage - 1, this.props.pageSize) // In API, pageNo starts from 0. But in the front end, pageNo starts from 1.
    }

    componentDidMount() {
        this.refreshPage()
    }

    loadProject(e) {
        let binaryFileId = e.target.dataset.binaryFileId
        if (!huahuoEngine.hasShape) {
            let project: any = this.projectInfoMap.get(Number(binaryFileId))
            if (project == null) {
                return
            }

            // Store is clean, directly load the project
            projectManager.loadFromServer(binaryFileId).then(() => {
                projectInfo.Setup(project.name, project.description, null)

                if (this.props.closeForm)
                    this.props.closeForm()
            })
        } else { // Ask the user if he/she wants to clear the current store. TODO: Can we merge the two stores in the future??
            HHToast.warn("Can't merge project, not implemented!!!")
        }
    }

    selectPage(e) {
        let pageNumber = Number(e.target.dataset.pageNumber) + 1
        this.state.currentPage = pageNumber
        this.refreshPage()
    }

    render() {
        this.projectInfoMap.clear()

        let binaryFileItems = []
        for (let binaryFile of this.state.binaryFiles) {
            let binaryUIItem =
                (<div key={binaryFile.id}>
                    <img
                        className="h-auto max-w-full rounded-lg hover:cursor-pointer"
                        src={api.getBinaryFileCoverPageUrl(binaryFile.id)}
                        data-binary-file-id={binaryFile.id}
                        onClick={this.loadProject.bind(this)}
                        alt="Alt Alt"
                    />
                </div>)
            binaryFileItems.push(binaryUIItem)

            this.projectInfoMap.set(binaryFile.id, binaryFile)
        }

        let currentClassName = "z-10 flex items-center justify-center px-3 h-8 leading-tight text-blue-600 border border-blue-300 bg-blue-50 hover:bg-blue-100 hover:text-blue-700 dark:border-gray-700 dark:bg-gray-700 dark:text-white"
        let notCurrentClassName = "flex items-center justify-center px-3 h-8 leading-tight text-gray-500 bg-white border border-gray-300 hover:bg-gray-100 hover:text-gray-700 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white"
        let pageLis = []
        for (let pageNo = 0; pageNo < this.state.totalPage; pageNo++) {
            let liClass = notCurrentClassName
            if (pageNo == this.state.currentPage - 1) {
                liClass = currentClassName
            }

            let liEle = (<li key={pageNo}>
                <a
                    href="#"
                    className={liClass}
                    data-page-number={pageNo}
                    onClick={this.selectPage.bind(this)}
                >
                    {pageNo + 1}
                </a>
            </li>)

            pageLis.push(liEle)
        }

        return (
            <div className="flex flex-col items-center justify-center mx-auto md:min-w-[800px]">
                <div
                    className="w-full bg-white rounded-lg drop-shadow-2xl dark:border md:mt-0 sm:max-w-md xl:p-0 dark:bg-gray-800 dark:border-gray-700">
                    <form className="p-4 space-y-4 divide-y divide-gray-300" action="#">
                        <div className="flex align-middle">
                            <h5 className="px-2 text-xl font-bold leading-tight tracking-tight text-gray-900 md:text-2xl dark:text-white">
                                {i18n.t(this.props.title)}
                            </h5>
                            <CloseBtn onclick={this.props?.closeForm}></CloseBtn>
                        </div>
                        <div className="grid grid-cols-2 md:grid-cols-3 gap-4">
                            {binaryFileItems}
                        </div>

                        <div aria-label="Page navigation" className="flex">
                            <div className="w-full"></div>
                            <ul className="flex items-center -space-x-px h-8 text-sm">
                                <li>
                                    <a
                                        href="#"
                                        className="flex items-center justify-center px-3 h-8 ml-0 leading-tight text-gray-500 bg-white border border-gray-300 rounded-l-lg hover:bg-gray-100 hover:text-gray-700 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white"
                                    >
                                        <span className="sr-only">Previous</span>
                                        <svg
                                            className="w-2.5 h-2.5"
                                            aria-hidden="true"
                                            xmlns="http://www.w3.org/2000/svg"
                                            fill="none"
                                            viewBox="0 0 6 10"
                                        >
                                            <path
                                                stroke="currentColor"
                                                strokeLinecap="round"
                                                strokeLinejoin="round"
                                                strokeWidth={2}
                                                d="M5 1 1 5l4 4"
                                            />
                                        </svg>
                                    </a>
                                </li>

                                {
                                    pageLis
                                }

                                <li>
                                    <a
                                        href="#"
                                        className="flex items-center justify-center px-3 h-8 leading-tight text-gray-500 bg-white border border-gray-300 rounded-r-lg hover:bg-gray-100 hover:text-gray-700 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white"
                                    >
                                        <span className="sr-only">Next</span>
                                        <svg
                                            className="w-2.5 h-2.5"
                                            aria-hidden="true"
                                            xmlns="http://www.w3.org/2000/svg"
                                            fill="none"
                                            viewBox="0 0 6 10"
                                        >
                                            <path
                                                stroke="currentColor"
                                                strokeLinecap="round"
                                                strokeLinejoin="round"
                                                strokeWidth={2}
                                                d="m1 9 4-4-4-4"
                                            />
                                        </svg>
                                    </a>
                                </li>
                            </ul>
                            <div className="w-full"></div>
                        </div>
                    </form>
                </div>
            </div>
        )
    }
}

export {ProjectListFormX}