class ValueChangeHandler{
    private valueChangeHandlersMap: Map<string, Map<number, Function>> = new Map()
    private valueChangeHandlersPreProcessorMap: Map<number, Function> = new Map()

    private handlerId: number = 0 // always increment

    callHandlers(propertyName: string, val: any) {
        if (this.valueChangeHandlersMap.has(propertyName)) {

            let propertyMap = this.valueChangeHandlersMap.get(propertyName)
            for (let [handlerId, handler] of propertyMap) {
                let preprocessor = this.valueChangeHandlersPreProcessorMap.get(handlerId)

                if(preprocessor)
                    handler(preprocessor(val))
                else
                    handler(val)
            }
        }
    }

    registerValueChangeHandler(valueName: string, preProcessor: Function = null) {
        return function (valueChangedHandler: Function) {
            if (!this.valueChangeHandlersMap.has(valueName)) {
                this.valueChangeHandlersMap.set(valueName, new Map<Number, Map<number, Function>>())
            }

            let id = this.handlerId
            this.valueChangeHandlersMap.get(valueName).set(id, valueChangedHandler)
            if(preProcessor)
                this.valueChangeHandlersPreProcessorMap.set(id, preProcessor)
            this.handlerId++
            return id;
        }.bind(this)
    }

    unregisterValueChangeHandler(valueName: string) {
        return function (handlerId: number) {
            if (this.valueChangeHandlersMap.has(valueName)) {
                let valueChangeHandlerMap = this.valueChangeHandlersMap.get(valueName)
                valueChangeHandlerMap.delete(handlerId)

                if(this.valueChangeHandlersPreProcessorMap.has(handlerId)){
                    valueChangeHandlerMap.delete(handlerId)
                }
            }
        }.bind(this)
    }

}

export {ValueChangeHandler}