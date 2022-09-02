import i18next from "i18next"

class I18nHandler{
    callBackFuncs: Array<Function> = new Array<Function>()

    constructor() {
        let _this = this
        i18next.on("loaded", function(loaded){
            if(loaded.hasOwnProperty("en")){
                for(let callbackFunc of _this.callBackFuncs){
                    callbackFunc()
                }
            }
        })
    }

    ExecuteAfterInited(callBack:Function){
        if(i18next.isInitialized){
            callBack()
        }else{
            this.callBackFuncs.push(callBack)
        }
    }

    t(...args):string{
        return i18next.t.apply(i18next, args)
    }
}

export {I18nHandler}