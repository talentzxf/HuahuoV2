declare var WrapperObject:any

function IsValidWrappedObject(obj):Boolean{
    if(!obj) return false
    if(!obj.hasOwnProperty('ptr')) return false
    if(!(obj instanceof WrapperObject))
        return false
    if(!obj.ptr)
        return false

    return true
}

export {IsValidWrappedObject}