declare var WrapperObject: any

function GetObjPtr(obj) {
    if (obj == null)
        return null

    if (obj.hasOwnProperty("ptr")) {
        return obj.ptr
    }
    if (obj.hasOwnProperty("rawObj"))
        return obj.rawObj.ptr

    return null
}

function IsValidWrappedObject(obj): Boolean {
    if (!obj) return false
    if (!obj.hasOwnProperty('ptr')) return false
    if (!(obj instanceof WrapperObject))
        return false
    if (!obj.ptr)
        return false

    return true
}

export {IsValidWrappedObject, GetObjPtr}