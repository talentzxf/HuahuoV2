import {LGraph} from "../Components/EventGraphComponent"

class LayerGraphWrapper {
    graphParams

    graph

    constructor(graphParams) {
        this.graphParams = graphParams

        this.graph = new LGraph()

        let graphJson = graphParams?.GetEventGraphJSON()
        if (graphJson && graphJson.length > 0) {
            let data = JSON.parse(graphJson)
            this.graph.configure(data)
        }

        this.graph["onAfterChange"] = this.saveGraph.bind(this)
    }

    getGraph() {
        return this.graph
    }

    saveGraph() {
        let graphString = JSON.stringify(this.graph.serialize())
        this.graphParams.SetEventGraphJSON(graphString)
    }
}

export {LayerGraphWrapper}