const SHORTKEYINTERVAL:number = 1 // Only within 1 seconds, count it as a valid series

enum ShortcutEventNames{
    UNDO,
    REDO,
    CUSTOM = 1000
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
        for(let property in obj){
            retKeyCode[property] = obj[property]
        }

        return retKeyCode
    }

    static fromKeyboardEvent(e:KeyboardEvent){
        return this.fromConfig(e)
    }

    toString():string{
        return JSON.stringify(this)
    }
}

// TODO: Do prefix search here.
class KeyCodeSeriesTree{
    children: Map<string, KeyCodeSeriesTree> = new Map<string, KeyCodeSeriesTree>()
    action: Function // If it's a leaf, store it's action here

    registerKeyCodeSeries(keyCodeConfigs:KeyCode[], action: Function){
        if(keyCodeConfigs.length == 0){ // No more key code, this is leaf
            this.action = action
        }else{
            let firstKeyCode:KeyCode = KeyCode.fromConfig(keyCodeConfigs[0])
            let newChild = new KeyCodeSeriesTree()
            newChild.registerKeyCodeSeries(keyCodeConfigs.slice(1), action)
            this.children.set(firstKeyCode.toString(), newChild)
        }
    }

    processKeyEvent(e: KeyCode): KeyCodeSeriesTree{
        let keyCode = KeyCode.fromConfig(e)
        if(this.children.has(keyCode.toString())){
            return this.children.get(keyCode.toString())
        }
        return null
    }
}

class ShortcutsManager{
    private rootKeyCodeSeries: KeyCodeSeriesTree = new KeyCodeSeriesTree()
    private currentKeyCodeSeries: KeyCodeSeriesTree = null
    private lastPressedTime: number = -1
    private registeredShortCutHandlers: Map<ShortcutEventNames, Array<Function>> = new Map();
    private inited: boolean = false

    init(){
        document.addEventListener("keypress", this.onKeyPressed.bind(this))

        // Add default shortcuts
        this.registerKeyCodeSeries([{
            ctrlKey: true,
            code:"z"
        }], this.triggerEventFunc(ShortcutEventNames.UNDO))

        // Add default shortcuts
        this.registerKeyCodeSeries([{
            ctrlKey: true,
            altKey: true,
            code:"z"
        }], this.triggerEventFunc(ShortcutEventNames.REDO))

        this.inited = true
    }

    registerKeyCodeSeries(keyCodeConfigs:KeyCode[], action: Function){
        this.rootKeyCodeSeries.registerKeyCodeSeries(keyCodeConfigs, action)
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
        if(this.currentKeyCodeSeries != null && this.currentKeyCodeSeries.action != null){
            this.currentKeyCodeSeries.action()
            this.currentKeyCodeSeries = null
        }

        this.lastPressedTime = Date.now()
    }
}

let shortcutsManager = new ShortcutsManager()
export {shortcutsManager, ShortcutEventNames}