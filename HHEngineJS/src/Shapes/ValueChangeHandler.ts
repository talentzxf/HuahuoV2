class ValueChangeHandler{
    public static wildCard = "*"

    private valueChangeHandlersMap: Map<string, Map<number, Function>> = new Map()
    private valueChangeHandlersPreProcessorMap: Map<number, Function> = new Map()

    private handlerId: number = 0 // always increment

    mergeMap(map1, map2): Map<number, Function>{
        if(map1 == null || map1.entries == null)
            return map2
        if(map2 == null || map2.entries == null)
            return map1

        return new Map([...map1.entries(), ...map2.entries()])
    }

    callHandlers(propertyName: string, val: any) {

        let mergedHandlerMap = this.mergeMap(this.valueChangeHandlersMap.get(propertyName), this.valueChangeHandlersMap.get(ValueChangeHandler.wildCard))
        if (mergedHandlerMap != null && mergedHandlerMap.size > 0) {

            let propertyMap = this.valueChangeHandlersMap.get(propertyName)
            let wildCardHandlerMap = this.valueChangeHandlersMap.get(ValueChangeHandler.wildCard)

            let mergedHandlerMap = this.mergeMap(propertyMap, wildCardHandlerMap)

            if(mergedHandlerMap){
                for (let [handlerId, handler] of mergedHandlerMap) {
                    let preprocessor = this.valueChangeHandlersPreProcessorMap.get(handlerId)

                    if(preprocessor)
                        handler(preprocessor(val))
                    else
                        handler(val)
                }
            }

        }
    }

    registerValueChangeHandler(valueNameString: string, preProcessor: Function = null) {
        return function (valueChangedHandler: Function): number[] {
            let valueNames = valueNameString.split("|") // Use | to subscribe multiple events.

            let returnIds = []
            for(let valueName of valueNames){
                if (!this.valueChangeHandlersMap.has(valueName)) {
                    this.valueChangeHandlersMap.set(valueName, new Map<Number, Map<number, Function>>())
                }

                let id = this.handlerId
                this.valueChangeHandlersMap.get(valueName).set(id, valueChangedHandler)
                if(preProcessor)
                    this.valueChangeHandlersPreProcessorMap.set(id, preProcessor)
                this.handlerId++

                returnIds.push(id)
            }

            return returnIds
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