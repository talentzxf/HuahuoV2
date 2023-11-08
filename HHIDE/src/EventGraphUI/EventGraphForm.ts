import {HHForm} from "../Utilities/HHForm";
import {CustomElement, getFullEventName} from "hhcommoncomponents";
import {CSSUtils} from "../Utilities/CSSUtils";
import {getEventCategoryMap} from "./Utils"
import {
    ActionDef,
    ActionNode,
    EventNode,
    getLiteGraphTypeFromPropertyType,
    huahuoEngine,
    LGraphCanvas,
    LiteGraph,
    NodeTargetType,
    PlayerActions,
    PropertyCategory,
    PropertyDef,
    renderEngine2D
} from "hhenginejs";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";

let CANVAS_WIDTH = 800
let CANVAS_HEIGHT = 600
let eventGraphPrefix = "eventgraph."

@CustomElement({
    selector: "hh-event-graph-form"
})
class EventGraphForm extends HTMLElement implements HHForm {
    canvas: HTMLCanvasElement

    selector: string;
    containerDiv: HTMLDivElement
    closeBtn: HTMLDivElement

    lcanvas: LGraphCanvas

    targetComponent

    setTargetComponent(targetComponent) {
        if (this.targetComponent != null && this.targetComponent != targetComponent) {
            if (this.setFrameIdHandler != -1) {
                huahuoEngine.getActivePlayer().getEventBus().removeEventHandler("Player", "setFrameId", this.setFrameIdHandler)
                this.setFrameIdHandler = -1

                let oldGraph = this.targetComponent.getGraph()
                oldGraph.onInputAdded = null
                oldGraph.onInputRemoved = null

                oldGraph.onInputNodeCreated = null
                oldGraph.onInputNodeRemoved = null

                for (let [nodeId, valueChangeHandler] of this.nodeIdvalueChangeHandlerIdMap) {
                    this.targetComponent.unregisterValueChangeHandlerFromAllValues(valueChangeHandler)
                }
            }
        }

        this.targetComponent = targetComponent

        this.initLGraph(this.canvas)
    }

    closeForm() {
        this.style.display = "none"
    }

    connectedCallback() {
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"
        this.containerDiv = document.createElement("div")
        this.containerDiv.innerHTML = CSSUtils.formStyle

        this.containerDiv.innerHTML += "<style>" +
            ".litegraph .dialog.settings {" +
            "height: calc( 50% - 20px );" +
            "max-width: 50%;" +
            "top: 30%" +
            "}" +
            "</style>"

        this.containerDiv.innerHTML += "<form id='eventGraphContainer' class='litegraph litegraph-editor'" +
            " style='width: fit-content; padding: 10px'> " +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='closeBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "</form>"

        let form = this.containerDiv.querySelector("form")
        this.closeBtn = this.containerDiv.querySelector("#closeBtn")
        this.closeBtn.onclick = this.closeForm.bind(this)

        this.canvas = document.createElement("canvas")
        this.canvas.width = CANVAS_WIDTH
        this.canvas.height = CANVAS_HEIGHT
        this.canvas.style.width = CANVAS_WIDTH + "px"
        this.canvas.style.height = CANVAS_HEIGHT + "px"
        form.appendChild(this.canvas)

        let resetScaleButton = document.createElement("button")
        resetScaleButton.innerText = i18n.t(eventGraphPrefix + "resetScale")
        resetScaleButton.style.width = "100px"
        resetScaleButton.style.padding = "0px"
        this.appendChild(this.containerDiv)

        let _this = this
        resetScaleButton.onclick = function (e) {
            _this.lcanvas.ds.changeScale(1.0);
            _this.lcanvas.graph.change()
            e.preventDefault()
        }

        form.appendChild(resetScaleButton)

        this.addEventListener("mousedown", this.onMouseDown.bind(this))
        this.addEventListener("mouseup", this.onMouseUp.bind(this))
    }

    isDragging: boolean = false
    mouseX: number = -1
    mouseY: number = -1

    onMouseDown(e: MouseEvent) {
        this.isDragging = true

        document.addEventListener("mousemove", this.onDrag.bind(this))

        this.mouseX = e.clientX
        this.mouseY = e.clientY
    }

    onDrag(e: MouseEvent) {
        if (this.isDragging && e.buttons == 1) { // Is dragging and left mouse is true.
            let offsetX = e.clientX - this.mouseX
            let offsetY = e.clientY - this.mouseY

            this.style.left = this.offsetLeft + offsetX + "px"
            this.style.top = this.offsetTop + offsetY + "px"
            this.mouseX = e.clientX
            this.mouseY = e.clientY
        }
    }

    onMouseUp() {
        this.isDragging = false
        this.mouseX = -1
        this.mouseY = -1

        document.removeEventListener("mousemove", this.onDrag.bind(this))
    }

    actionCallBack(value, event, mouseEvent, contextMenu, extraOptions, actionDef: ActionDef, type: NodeTargetType, additionalInfo = null) {
        let first_event = contextMenu.getFirstEvent();
        let graph = this.lcanvas.graph
        let lcanvas = this.lcanvas
        graph.beforeChange()

        let node = LiteGraph.createNode(value.value) as ActionNode
        if (node) {
            let paramDefs = actionDef.paramDefs
            for (let paramDef of paramDefs) {
                let inputSlot = node.addInput(paramDef.paramName, getLiteGraphTypeFromPropertyType(paramDef.paramType))
                node.addParameterIndexSlotMap(paramDef.paramIndex, inputSlot)
            }

            node.pos = lcanvas.convertEventToCanvasOffset(first_event)
            lcanvas.graph.add(node)

            node.setEventTargetType(type, additionalInfo)
            node.setActionDef(actionDef)

            if (extraOptions) {
                if (extraOptions.isFrom) {
                    extraOptions.nodeFrom.connectByType(extraOptions.iSlotConn, node, extraOptions.fromSlotType);
                } else {
                    if (extraOptions.nodeTo) {
                        extraOptions.nodeTo.connectByTypeOutput(extraOptions.iSlotConn, node, extraOptions.fromSlotType);
                    }
                }
            }

        }

        graph.afterChange()
    }

    hasComponentActionMenu(): boolean {
        let baseShape = this.targetComponent.baseShape
        if (baseShape == null) {
            return false
        }

        let components = baseShape.getComponents()
        if (components.length > 0) {
            for (let component of components) {
                for (let actionDef of component.getActionDefs()) {
                    return true
                }
            }
        }

        return false
    }

    componentActionMenu(node, options, e, prev_menu, extraOptions) {
        if (!this.lcanvas)
            return

        if (!this.lcanvas.graph)
            return

        let ref_window = this.lcanvas.getCanvasWindow()

        let baseShape = this.targetComponent.baseShape
        let _this = this
        let entries = []

        let components = baseShape.getComponents()

        if (components.length > 0) {
            for (let component of components) {
                component.getActionDefs().forEach((actionDef) => {
                    let entry = {
                        value: "actions/actionNode",
                        content: component.getTypeName() + "/" + actionDef.actionName,
                        has_submenu: false,
                        callback: function (value, event, mouseEvent, contextMenu) {
                            _this.actionCallBack(value, event, mouseEvent, contextMenu, extraOptions, actionDef, NodeTargetType.COMPONENT, {
                                componentId: component.getRawObject().GetIdxInShape()
                            })
                        }
                    }

                    entries.push(entry)
                })
            }


            new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
        }
    }

    actionMenu(node, options, e, prev_menu, extraOptions = null) {
        if (!this.lcanvas)
            return

        if (!this.lcanvas.graph)
            return

        let ref_window = this.lcanvas.getCanvasWindow()

        let _this = this

        let entries = [
            {
                content: i18n.t("eventgraph.addActions"),
                has_submenu: true,
                callback: (node, options, e, prev_menu, myExtraOptions) => {
                    let resultExtraOptions = Object.assign({}, myExtraOptions, extraOptions)
                    _this.shapeActionMenu(node, options, e, prev_menu, resultExtraOptions)
                }
            }
        ]

        if (this.targetComponent.hasOwnProperty("playerAction")) {
            entries.push({
                content: i18n.t("eventgraph.addPlayerActions"),
                has_submenu: true,
                callback: (node, options, e, prev_menu, myExtraOptions) => {
                    let resultExtraOptions = Object.assign({}, myExtraOptions, extraOptions)
                    _this.playerActionMenu(node, options, e, prev_menu, resultExtraOptions)
                }
            })
        }

        if (this.hasComponentActionMenu()) {
            entries.push({
                content: i18n.t("eventgraph.addComponentGraphAction"),
                has_submenu: true,
                callback: (node, options, e, prev_menu, myExtraOptions) => {
                    let resultExtraOptions = Object.assign({}, myExtraOptions, extraOptions)
                    _this.componentActionMenu(node, options, e, prev_menu, resultExtraOptions)
                }
            })
        }

        new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
    }

    playerActionMenu(node, options, e, prev_menu, extraOptions) {
        if (!this.lcanvas)
            return

        if (!this.lcanvas.graph)
            return

        let ref_window = this.lcanvas.getCanvasWindow()

        let _this = this

        let baseActor = this.targetComponent.playerAction
        let actionDefs = baseActor.getActionDefs()

        let entries = []
        actionDefs.forEach((actionDef) => {
            let entry = {
                value: "actions/actionNode",
                content: actionDef.actionName,
                has_submenu: false,
                callback: function (value, event, mouseEvent, contextMenu) {
                    _this.actionCallBack(value, event, mouseEvent, contextMenu, extraOptions, actionDef, NodeTargetType.PLAYER)
                }
            }
            entries.push(entry)
        })

        new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
    }

    shapeActionMenu(node, options, e, prev_menu, extraOptions) {
        if (!this.lcanvas)
            return

        if (!this.lcanvas.graph)
            return

        let ref_window = this.lcanvas.getCanvasWindow()

        let _this = this
        let baseActor = this.targetComponent.getBaseActor()
        let actionDefs = baseActor.getActionDefs()

        let entries = []
        actionDefs.forEach((actionDef) => {
            let entry = {
                value: "actions/actionNode",
                content: actionDef.actionName,
                has_submenu: false,
                callback: function (value, event, mouseEvent, contextMenu) {
                    _this.actionCallBack(value, event, mouseEvent, contextMenu, extraOptions, actionDef, NodeTargetType.SHAPE)
                }
            }

            entries.push(entry)
        })

        new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
    }


    eventListenerMenu(node, options, e, prev_menu, callback) {
        if (!this.lcanvas)
            return

        if (!this.lcanvas.graph)
            return

        let ref_window = this.lcanvas.getCanvasWindow()

        // Map from the event name to the object that triggers the event.
        // If the value is null, the event is triggered in global eventBus.
        let eventNameTypeMap = new Map
        let eventNameAdditionalInfoMap = new Map

        let player = huahuoEngine.getActivePlayer()
        // Build up player events
        let events = player.getEventBus().getAllEvents()
        for (let eventName of events) {
            eventNameTypeMap.set(eventName, NodeTargetType.PLAYER)
        }

        // Build up renderEngine events
        let renderEngineEvents = renderEngine2D.getEventBus().getAllEvents()
        for (let eventName of renderEngineEvents) {
            eventNameTypeMap.set(eventName, NodeTargetType.CANVAS)
        }

        if (this.targetComponent.baseShape) {
            // Build up component events
            this.targetComponent.baseShape.getComponents().forEach((component) => {
                let componentEvents = huahuoEngine.getEvent(component).getEvents()
                for (let eventName of componentEvents) {
                    eventNameAdditionalInfoMap.set(eventName, {
                        "componentId": component.getRawObject().GetIdxInShape()
                    })
                    eventNameTypeMap.set(eventName, NodeTargetType.COMPONENT)
                }
            })

            // Build up self events.
            let localEvents = huahuoEngine.getEvent(this.targetComponent.baseShape).getEvents()

            for (let eventName of localEvents) {
                eventNameTypeMap.set(eventName, NodeTargetType.SHAPE)
            }
        }

        let selfEvents = huahuoEngine.getEvent(this.targetComponent).getEvents()
        for (let eventName of selfEvents) {
            eventNameTypeMap.set(eventName, NodeTargetType.GRAPHCOMPONENT)
        }

        let namespaceCategories = getEventCategoryMap(eventNameTypeMap, eventNameAdditionalInfoMap)

        let entries = []

        let _this = this
        namespaceCategories.forEach((eventObjects, namespace) => {
            let entry = {
                value: namespace,
                content: i18n.t(eventGraphPrefix + namespace),
                has_submenu: true,
                callback: function (value, event, mouseEvent, contextMenu) {
                    let eventEntries = []

                    eventObjects.forEach((eventObject: object) => {
                        let eventName = i18n.t(eventGraphPrefix + eventObject["eventName"])
                        let eventEntry = {
                            value: "events/eventNode",
                            content: eventName,
                            has_submenu: false,
                            callback: function (value, event, mouseEvent, contextMenu) {
                                let fullEventName = getFullEventName(namespace, eventObject["eventName"])

                                let first_event = contextMenu.getFirstEvent();
                                let graph = _this.lcanvas.graph
                                let lcanvas = _this.lcanvas
                                graph.beforeChange()

                                let node = LiteGraph.createNode(value.value) as EventNode
                                if (node) {
                                    let paramDefs = huahuoEngine.getEventBus(eventObject["eventSource"]).getEventParameters(fullEventName) || []
                                    for (let paramDef of paramDefs) {
                                        let outputSlot = node.addOutput(paramDef.parameterName, getLiteGraphTypeFromPropertyType(paramDef.parameterType), {
                                            label: i18n.t(eventGraphPrefix + paramDef.parameterName)
                                        })
                                        node.addParameterIndexSlotMap(paramDef.paramIndex, outputSlot)
                                    }

                                    node.pos = lcanvas.convertEventToCanvasOffset(first_event)
                                    lcanvas.graph.add(node)

                                    node.setEventTargetType(eventObject["eventType"], eventObject["additionalInfo"])

                                    let splitedStrings: string[] = fullEventName.split(":")
                                    let resultTitle = ""
                                    let idx = 0
                                    for (let str of splitedStrings) {
                                        resultTitle += i18n.t(eventGraphPrefix + str)

                                        idx++
                                        if (idx < splitedStrings.length)
                                            resultTitle += ":"
                                    }

                                    node.setFullEventName(fullEventName, resultTitle)
                                }

                                if (callback)
                                    callback(node)

                                graph.afterChange()
                            }
                        }
                        eventEntries.push(eventEntry)
                    })

                    new LiteGraph.ContextMenu(eventEntries, {event: e, parentMenu: contextMenu}, ref_window)
                }
            }

            entries.push(entry)
        })

        new LiteGraph.ContextMenu(entries, {event: e, parentMenu: prev_menu}, ref_window)
    }

    onInputAdded(inputName: string, inputType: string) {
        if (!this.targetComponent.addInput(inputName, inputType)) {
            return;
        }

        this.targetComponent.updateComponentPropertySheet(this.targetComponent.baseShape.getPropertySheet())
        // Refresh the component inspector
        IDEEventBus.getInstance().emit(EventNames.COMPONENTCHANGED, this.targetComponent.baseShape)
    }

    onInputRemoved(inputName: string) {
        console.log("Input removed")
    }

    // From NodeId -> HandlerId map.
    nodeIdvalueChangeHandlerIdMap: Map<number, number> = new Map

    setNodeValue(node, value) {
        node.graph.beforeChange()

        let prevSetInputValueFunction = node.graph.setInputValueFunction
        node.graph.setInputValueFunction = null

        node.setProperty("value", value) // Avoid impacting real component value.

        node.graph.setInputValueFunction = prevSetInputValueFunction

        node.graph.change()
    }

    onInputNodeCreated(node) {
        let propertyName = node.properties.name

        let nodeId = node.id

        let _this = this
        if (this.targetComponent.hasOwnProperty(propertyName)) {
            let valueChangeHandlerId = this.targetComponent.registerValueChangeHandler(propertyName, (value) => {
                let inputNode = _this.targetComponent.getGraph().getNodeById(nodeId)
                if (inputNode != null)
                    _this.setNodeValue(inputNode, value)
                else{
                    let valueChangeHandler = this.nodeIdvalueChangeHandlerIdMap.get(node.id)
                    this.targetComponent.unregisterValueChangeHandlerFromAllValues(valueChangeHandler)
                }
            })

            this.nodeIdvalueChangeHandlerIdMap.set(nodeId, valueChangeHandlerId)

            if (node.properties.value != this.targetComponent[propertyName]) {
                this.setNodeValue(node, this.targetComponent[propertyName])
            }
        }
    }

    setFrameIdHandler = -1

    initLGraph(canvas: HTMLCanvasElement) {
        let graph = this.targetComponent.getGraph()

        this.setFrameIdHandler = huahuoEngine.getActivePlayer().getEventBus().addEventHandler("Player", "setFrameId", () => {
            let inputNodes = graph.findNodesByType("graph/input")
            for (let inputNode of inputNodes) {
                let propertyName = inputNode.properties.name

                if (inputNode.properties.value != this.targetComponent[propertyName]) {
                    this.setNodeValue(inputNode, this.targetComponent[propertyName])
                }
            }
        })

        graph.onInputAdded = this.onInputAdded.bind(this)
        graph.onInputRemoved = this.onInputRemoved.bind(this)

        graph.onInputNodeCreated = this.onInputNodeCreated.bind(this)
        // graph.onInputNodeRemoved = this.onInputNodeRemoved.bind(this)

        // Bind all current input nodes
        let inputNodes = graph.findNodesByType("graph/input")
        for (let inputNode of inputNodes) {
            this.onInputNodeCreated(inputNode)
        }

        if (this.lcanvas == null) {
            this.lcanvas = new LGraphCanvas(canvas, graph, {autoresize: false})
            let _this = this
            let actionMenuOption = {
                content: i18n.t("eventgraph.addGraphAction"),
                has_submenu: true,
                callback: _this.actionMenu.bind(_this)
            }

            this.lcanvas.getExtraMenuOptions = function () {
                let options = [
                    {
                        content: i18n.t("eventgraph.addEventListener"),
                        has_submenu: true,
                        callback: _this.eventListenerMenu.bind(_this)
                    },
                ]

                options.push(actionMenuOption)

                return options
            }

            this.lcanvas.show_inputs_panel = true

            // var node_const = LiteGraph.createNode("basic/const");
            // node_const.pos = [200, 200];
            // this.graph.add(node_const);
            // node_const.setValue(4.5);
            //
            // var node_watch = LiteGraph.createNode("basic/watch");
            // node_watch.pos = [700, 200];
            // this.graph.add(node_watch);
            //
            // node_const.connect(0, node_watch, 0);

            LiteGraph["release_link_on_empty_shows_menu"] = true

            this.lcanvas.title_texts.GraphInputs = i18n.t("eventgraph.GraphInputs")

            this.lcanvas.slot_types_default_out["_event_"] = [actionMenuOption]

        } else
            this.lcanvas.setGraph(graph)
    }

}

export {EventGraphForm}