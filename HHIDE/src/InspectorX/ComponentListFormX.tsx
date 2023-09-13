import * as React from "react"
import {CloseBtn} from "../UIComponents/CloseBtn";
import {FormProps} from "../Utilities/FormManager";
import {huahuoEngine} from "hhenginejs";
import {AddComponentCommand} from "../RedoUndo/AddComponentCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {EditorComponentProxy} from "../ComponentProxy/ComponentProxy";

type ComponentListFormProps = FormProps & {
    componentNames: string[],
    targetObject: any
}

class ComponentListFormX extends React.Component<ComponentListFormProps, any> {
    onAddComponentClicked(e) {
        e.preventDefault()
        let componentName = e.currentTarget.dataset.componentName
        let newComponent = huahuoEngine.produceObject(componentName)

        let proxiedComponent = EditorComponentProxy.CreateProxy(newComponent)

        let addComponentCommand = new AddComponentCommand(this.props.targetObject, proxiedComponent)
        addComponentCommand.DoCommand()

        undoManager.PushCommand(addComponentCommand)

        this.props.closeForm()
    }

    getClassName() {
        return "block w-full cursor-pointer rounded-lg p-4 text-left transition duration-500 hover:bg-primary-100 hover:text-neutral-500 focus:ring-0 dark:hover:bg-neutral-600 dark:hover:text-neutral-200"
    }

    render() {
        return (
            <div className="flex flex-col items-center justify-center mx-auto">
                <div
                    className="w-full bg-white rounded-lg drop-shadow-2xl dark:border md:mt-0 sm:max-w-md xl:p-0 dark:bg-gray-800 dark:border-gray-700">
                    <form className="p-4 space-y-4 divide-y divide-gray-300" action="#">
                        <div className="flex align-middle">
                            <h5 className="px-2 text-xl font-bold leading-tight tracking-tight text-gray-900 md:text-2xl dark:text-white">
                                {i18n.t("ComponentList")}
                            </h5>
                            <CloseBtn onclick={this.props?.closeForm}></CloseBtn>
                        </div>
                        <ul className="grid grid-cols-2">
                            {
                                this.props.componentNames.map((item) => {
                                    return (<button key={Math.random()} onClick={this.onAddComponentClicked.bind(this)}
                                                    className={this.getClassName()} data-component-name={item}>{i18n.t(item)}</button>)
                                })
                            }
                        </ul>
                    </form>
                </div>
            </div>
        )
    }
}

export {ComponentListFormX}