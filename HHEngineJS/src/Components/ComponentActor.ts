class ComponentActor{
    fieldValueMap: Map<string, object> = new Map()

    setField(fieldName: string, value: object){
        this.fieldValueMap.set(fieldName, value)
    }

    hasField(fieldName: string){
        return this.fieldValueMap.has(fieldName)
    }

    getField(fieldName: string){
        if(!this.fieldValueMap.has(fieldName))
            return null

        return this.fieldValueMap.get(fieldName)
    }

    reset(){
        this.fieldValueMap = new Map()
    }
}

export {ComponentActor}