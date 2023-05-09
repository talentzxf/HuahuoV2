class Utils{
    static isValidGUID(guid: string) {
        if(guid.length <= 0)
            return false

        for (let char of guid) {
            if (char != '0')
                return true
        }

        return false
    }
}


export {Utils}