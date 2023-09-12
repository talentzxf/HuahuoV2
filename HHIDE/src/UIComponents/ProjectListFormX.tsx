import * as React from "react"
import {CloseBtn} from "./CloseBtn";
import {FormProps} from "../Utilities/FormManager";

type ProjectListProps = FormProps & {
    updateListFunction: Function
    title: string
    totalPage: number
    pageNo: number
    pageSize: number
}

class ProjectListFormX extends React.Component<any, any> {
    render() {
        return (
            <div className="flex flex-col items-center justify-center mx-auto">
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
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-1.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-2.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-3.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-4.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-5.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-6.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-7.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-8.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-9.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-10.jpg"
                                    alt=""
                                />
                            </div>
                            <div>
                                <img
                                    className="h-auto max-w-full rounded-lg"
                                    src="https://flowbite.s3.amazonaws.com/docs/gallery/square/image-11.jpg"
                                    alt=""
                                />
                            </div>
                        </div>

                        <nav aria-label="Page navigation example">
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
                                <li>
                                    <a
                                        href="#"
                                        className="flex items-center justify-center px-3 h-8 leading-tight text-gray-500 bg-white border border-gray-300 hover:bg-gray-100 hover:text-gray-700 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white"
                                    >
                                        1
                                    </a>
                                </li>
                                <li>
                                    <a
                                        href="#"
                                        className="flex items-center justify-center px-3 h-8 leading-tight text-gray-500 bg-white border border-gray-300 hover:bg-gray-100 hover:text-gray-700 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white"
                                    >
                                        2
                                    </a>
                                </li>
                                <li>
                                    <a
                                        href="#"
                                        aria-current="page"
                                        className="z-10 flex items-center justify-center px-3 h-8 leading-tight text-blue-600 border border-blue-300 bg-blue-50 hover:bg-blue-100 hover:text-blue-700 dark:border-gray-700 dark:bg-gray-700 dark:text-white"
                                    >
                                        3
                                    </a>
                                </li>
                                <li>
                                    <a
                                        href="#"
                                        className="flex items-center justify-center px-3 h-8 leading-tight text-gray-500 bg-white border border-gray-300 hover:bg-gray-100 hover:text-gray-700 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white"
                                    >
                                        4
                                    </a>
                                </li>
                                <li>
                                    <a
                                        href="#"
                                        className="flex items-center justify-center px-3 h-8 leading-tight text-gray-500 bg-white border border-gray-300 hover:bg-gray-100 hover:text-gray-700 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white"
                                    >
                                        5
                                    </a>
                                </li>
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
                        </nav>
                    </form>
                </div>
            </div>
        )
    }
}

export {ProjectListFormX}