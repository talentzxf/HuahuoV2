function capitalizeFirstLetter(str){
    if(str.length == 0)
        return ""

    return str.charAt(0).toUpperCase() + str.slice(1);
}

function getFieldNameFromGetterName(str){
    if(str.length <= 4)
        throw "String length is less than 4"
    if(!str.startsWith("get")){
        throw "String is not started with get"
    }

    let fieldName = str.substring(3)
    fieldName[0] = fieldName[0].toLowerCase()

    return fieldName
}

export {capitalizeFirstLetter, getFieldNameFromGetterName}