enum ShortcutEvents{
    UNDO,
    REDO
}

class KeyCode{
    ctrlKey: boolean
    altKey: boolean
    shiftKey: boolean
    charCode: string

    static fromKeyboardEvent(e:KeyboardEvent){
        let retKeyCode = new KeyCode()
        if(e.ctrlKey){
            retKeyCode.ctrlKey = true
        }

        if(e.altKey){
            retKeyCode.altKey = true
        }

        if(e.shiftKey){
            retKeyCode.shiftKey = true
        }

        if(e.key && e.key){

        }
    }
}

class KeyCodeSeries{
    keycodes:Array<KeyCode> = new Array<KeyCode>()
}

class ShortcutsManager{
    shortCuts: Array<KeyCodeSeries> = new Array<KeyCodeSeries>()

    init(){
        document.addEventListener("keypress", this.onKeyPressed.bind(this))
    }

    onKeyPressed(){

    }
}

let shortcutsManager = new ShortcutsManager()
export {shortcutsManager}