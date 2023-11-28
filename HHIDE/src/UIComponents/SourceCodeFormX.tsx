import * as React from "react"
import {CodeComponent} from "hhenginejs";
import {CloseBtn} from "./CloseBtn";
import {FormProps} from "../Utilities/FormManager";
import * as monaco from "monaco-editor"

let EDITOR_WIDTH = 800
let EDITOR_HEIGHT = 600

type SourceCodeFormXProps = FormProps & {
    targetComponent: CodeComponent
}

type SourceCodeFormXState = {
    left: string,
    top: string,
    width: string,
    height: string
}

let exampleCode = "class Handler{\n" +
    "\tconstruct(shapeActor, eventRegisters){\n" +
    "\t\tthis.shapeActor = shapeActor\n" +
    "\t\tthis.eventRegisters = eventRegisters\n" +
    "\t\t\n" +
    "\t\tconsole.log(\"HiHi\")\n" +
    "\t}\n" +
    "\t\n" +
    "\tonStart(){\n" +
    "\t\tconsole.log(\"onStart\")\n" +
    "\t}\n" +
    "\t\n" +
    "\tonUpdate(){\n" +
    "\t\tconsole.log(\"onUpdate\")\n" +
    "\t}\n" +
    "}\n" +
    "\n" +
    "RegisterClass(Handler)"

class SourceCodeFormX extends React.Component<SourceCodeFormXProps, any> {
    state: SourceCodeFormXState = {
        left: "-" + EDITOR_WIDTH / 2 + "px",
        top: "-" + EDITOR_HEIGHT / 2 + "px",
        width: EDITOR_WIDTH + "px",
        height: EDITOR_HEIGHT + "px"
    }
    mouseX: number = -1
    mouseY: number = -1
    isDragging: boolean = false

    onDragFunc = this.onDrag.bind(this)

    rootDivRef
    editorDivRef

    monacoEditor = null

    constructor(props) {
        super(props);

        this.rootDivRef = React.createRef()
        this.editorDivRef = React.createRef()
    }

    onDrag(e) {
        if (this.isDragging && e.buttons == 1) { // Is dragging and left mouse is true.
            let offsetX = e.clientX - this.mouseX
            let offsetY = e.clientY - this.mouseY

            this.state.left = this.rootDivRef.current.offsetLeft + offsetX + "px"
            this.state.top = this.rootDivRef.current.offsetTop + offsetY + "px"
            this.mouseX = e.clientX
            this.mouseY = e.clientY

            this.setState(this.state)
        }
    }

    onMouseDown(e) {
        this.isDragging = true
        document.addEventListener("mousemove", this.onDragFunc)

        this.mouseX = e.clientX
        this.mouseY = e.clientY
    }

    onMouseUp(e) {
        this.isDragging = false
        this.mouseX = -1
        this.mouseY = -1
        document.removeEventListener("mousemove", this.onDragFunc)
    }

    componentDidMount() {
        if(this.monacoEditor == null){
            this.monacoEditor = monaco.editor.create(this.editorDivRef.current, {
                value: exampleCode,
                language: "typescript"
            })

            let currentCode = this.props.targetComponent.sourceCode
            if(currentCode != null && currentCode.length > 0){
                this.monacoEditor.setValue(currentCode)
            }
        }else{
            this.monacoEditor.setValue(this.props.targetComponent.sourceCode)
        }
    }

    saveCode(){
        this.props.targetComponent.sourceCode = this.monacoEditor.getValue()
    }

    render() {
        return (
            <div ref={this.rootDivRef} className="select-none flex flex-col items-center justify-center mx-auto"
                 style={{
                     position: "absolute",
                     left: this.state.left,
                     top: this.state.top,
                     width: "fit-content",
                     height: "fit-content"
                 }}
                 onMouseDown={this.onMouseDown.bind(this)} onMouseUp={this.onMouseUp.bind(this)}>
                <div
                    className="bg-white rounded-lg drop-shadow-2xl dark:border md:mt-0 xl:p-0
                        dark:bg-gray-800 dark:border-gray-700" style={{
                    width: "fit-content",
                    height: "fit-content"
                }}>
                    <form className="p-4 space-y-4 divide-y divide-gray-300" style={{
                        width: "fit-content",
                        height: "fit-content"
                    }} action="#">
                        <div className="flex align-middle">
                            <h5 className="px-2 text-xl font-bold leading-tight tracking-tight text-gray-900 md:text-2xl dark:text-white">
                                {i18n.t("inspector.property.sourceCode")}
                            </h5>
                            <CloseBtn onclick={
                                () => {
                                    this.saveCode()
                                    
                                    if(this.monacoEditor)
                                        this.monacoEditor.dispose()

                                    this.props?.closeForm()
                                }}></CloseBtn>
                        </div>
                        <div style={{
                            width: this.state.width,
                            height: this.state.height
                        }} ref={this.editorDivRef}>
                        </div>
                    </form>
                </div>
            </div>
        );
    }
}

export {SourceCodeFormX}