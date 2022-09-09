import {CustomElement} from "../CustomComponent";
import "../../css/toast.css"

enum TOAST_POSITION{
    LEFTUP,
    LEFTDOWN,
    RIGHTUP,
    RIGHTDOWN
}

enum TOAST_LEVEL{
    INFO,
    WARN,
    ERROR
}

@CustomElement({
     selector: "hh-toast",
})
class HHToast extends HTMLElement{
    connectedCallback(){
        let position = this.getAttribute("position")
    }

    setText(level:TOAST_LEVEL, text:string){
        this.innerText = text

        switch (level){
            case TOAST_LEVEL.INFO:
                this.className = "info"
                break;
            case TOAST_LEVEL.WARN:
                this.className = "warn"
                break;
            case TOAST_LEVEL.ERROR:
                this.className = "error"
                break;
        }

        this.classList.add("show")

        let _this = this
        setTimeout(function(){
            _this.classList.remove("show")
            }, 2900
        )
    }

    static createOrGetHHToast():HHToast{
        let hhtoast = document.querySelector("hh-toast")
        if(!hhtoast){
            hhtoast = document.createElement("hh-toast")
            document.body.appendChild(hhtoast)
        }

        return hhtoast as HHToast
    }

    static info(text:string){
        let hhtoast:HHToast = this.createOrGetHHToast()
        hhtoast.setText(TOAST_LEVEL.INFO, text)
    }

    static warn(text:string){
        let hhtoast:HHToast = this.createOrGetHHToast()
        hhtoast.setText(TOAST_LEVEL.WARN, text)
    }

    static error(text:string){
        let hhtoast:HHToast = this.createOrGetHHToast()
        hhtoast.setText(TOAST_LEVEL.ERROR, text)
    }
}

export {HHToast}