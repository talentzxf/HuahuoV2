const SHORTKEYINTERVAL:number = 1 // Only within 1 seconds, count it as a valid series

enum ShortcutEventNames{
    UNDO,
    REDO
}

function isAlphaNum(keyCode){
    if(keyCode >= "0".charCodeAt(0) && keyCode <= "z".charCodeAt(0)){
        return true
    }
    return false
}

class KeyCode{
    ctrlKey ?: boolean = false
    altKey ?: boolean = false
    shiftKey ?: boolean = false
    code: string

    static fromConfig(obj){
        let retKeyCode = new KeyCode()
        if(obj.ctrlKey){
            retKeyCode.ctrlKey = true
        }

        if(obj.altKey){
            retKeyCode.altKey = true
        }

        if(obj.shiftKey){
            retKeyCode.shiftKey = true
        }

        // Have to press a "solid" key to make this keycode valid
        if(obj.key && isAlphaNum(obj.key)){
            retKeyCode.code = obj.key
        }else{
            return null
        }

        return retKeyCode
    }

    static fromKeyboardEvent(e:KeyboardEvent){
        return this.fromConfig(e)
    }
}

// TODO: Do prefix search here.
class KeyCodeSeriesTree{
    children: Map<KeyCode, KeyCodeSeriesTree> = new Map<KeyCode, KeyCodeSeriesTree>()
    action: Function // If it's a leaf, store it's action here

    registerKeyCodeSeries(keyCodeConfigs:KeyCode[], action: Function){
        if(keyCodeConfigs.length == 0){ // No more key code, this is leaf
            this.action = action
        }else{
            let firstKeyCode = keyCodeConfigs[0]
            let newChild = new KeyCodeSeriesTree()
            newChild.registerKeyCodeSeries(keyCodeConfigs.slice(1), action)
            this.children.set(firstKeyCode, newChild)
        }
    }

    processKeyEvent(e: KeyCode): KeyCodeSeriesTree{
        if(this.children.has(e)){
            return this.children.get(e)
        }
        return null
    }
}

class ShortcutsManager{
    rootKeyCodeSeries: KeyCodeSeriesTree = new KeyCodeSeriesTree()
    currentKeyCodeSeries: KeyCodeSeriesTree = null
    lastPressedTime: number = -1

    registeredShortCutHandlers: Map<ShortcutEventNames, Array<Function>> = new Map();

    init(){
        document.addEventListener("keypress", this.onKeyPressed.bind(this))

        // Add default shortcuts
        this.rootKeyCodeSeries.registerKeyCodeSeries([{
            ctrlKey: true,
            code:"z"
        }], this.triggerEventFunc(ShortcutEventNames.UNDO))

        // Add default shortcuts
        this.rootKeyCodeSeries.registerKeyCodeSeries([{
            ctrlKey: true,
            altKey: true,
            code:"z"
        }], this.triggerEventFunc(ShortcutEventNames.REDO))
    }

    registerShortcutHandler(eventName:ShortcutEventNames, callback: Function){
        if(!this.registeredShortCutHandlers.has(eventName)){
            this.registeredShortCutHandlers.set(eventName, new Array<Function>())
        }

        this.registeredShortCutHandlers.get(eventName).push(callback)
    }

    triggerEventFunc(eventName: ShortcutEventNames){
        let _this = this
        return function(){
            for(let handler of _this.registeredShortCutHandlers.get(eventName)){
                handler()
            }
        }
    }

    keyEventToKeyCode(e:KeyboardEvent): KeyCode{
        return {
            ctrlKey: e.ctrlKey,
            shiftKey: e.shiftKey,
            altKey: e.altKey,
            code : e.key
        }
    }

    onKeyPressed(e: KeyboardEvent){
        if(this.currentKeyCodeSeries == null || (Date.now() - this.lastPressedTime) > SHORTKEYINTERVAL * 1000){
            // Reset back to root
            this.currentKeyCodeSeries = this.rootKeyCodeSeries
        }

        this.currentKeyCodeSeries = this.currentKeyCodeSeries.processKeyEvent(this.keyEventToKeyCode(e))
        if(this.currentKeyCodeSeries.action != null){
            this.currentKeyCodeSeries.action()
            this.currentKeyCodeSeries = null
        }

        this.lastPressedTime = Date.now()
    }
}

let shortcutsManager = new ShortcutsManager()
export {shortcutsManager, ShortcutEventNames}