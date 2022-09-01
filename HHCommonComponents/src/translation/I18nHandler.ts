import i18next from "i18next"

let i18n = (window as any).i18next
class I18nHandler{
    callBackFuncs: Array<Function> = new Array<Function>()

    constructor() {
        let _this = this
        i18n.on("loaded", function(){
            for(let callbackFunc of _this.callBackFuncs){
                callbackFunc()
            }
        })
    }

    ExecuteAfterInited(callBack:Function){
        if(i18n.isInitialized){
            callBack()
        }else{
            this.callBackFuncs.push(callBack)
        }
    }

    t(input:string):string{
        return i18next.t(input)
    }
}

export {I18nHandler}
