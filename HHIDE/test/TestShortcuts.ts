import {ShortcutEventNames, shortcutsManager} from "../src/Shortcuts/ShortcutsManager";
import "mocha"
import {assert} from "chai"

describe('TestShortcuts manager', ()=> {
    it("Test Ctrl-Z", ()=>{
        let hasUndone = false

        shortcutsManager.init()

        shortcutsManager.registerShortcutHandler(ShortcutEventNames.UNDO, ()=>{
            hasUndone = true
        })

        let kbEvent = new KeyboardEvent("keydown", {
            ctrlKey: true,
            key: "z"
        })
        shortcutsManager.onKeyPressed(kbEvent)

        assert(hasUndone)

        hasUndone = false
        let hasDone = false
        let kbEvent2 = new KeyboardEvent("keydown", {
            ctrlKey: true,
            altKey: true,
            key: "z"
        })
        shortcutsManager.registerShortcutHandler(ShortcutEventNames.REDO, ()=>{
            hasDone = true
        })
        shortcutsManager.onKeyPressed(kbEvent2)
        assert(hasUndone == false && hasDone)
    })

    it("Test Custom events", ()=>{
        shortcutsManager.init()

        let executed = false
        shortcutsManager.registerKeyCodeSeries([{code:"a"}, {code:"b"}, {code:"c"},{code:"d"}], ()=>{
            executed = true
        })

        let kbEvent1 = new KeyboardEvent("keydown", {
            key: "a"
        })
        let kbEvent2 = new KeyboardEvent("keydown", {
            key: "b"
        })
        let kbEvent3 = new KeyboardEvent("keydown", {
            key: "c"
        })
        let kbEvent4 = new KeyboardEvent("keydown", {
            key: "d"
        })

        shortcutsManager.onKeyPressed(kbEvent1)
        shortcutsManager.onKeyPressed(kbEvent2)
        shortcutsManager.onKeyPressed(kbEvent3)
        shortcutsManager.onKeyPressed(kbEvent4)

        assert(executed)

        executed = false
        let kbEvent5 = new KeyboardEvent("keydown", {
            key: "e"
        })

        shortcutsManager.onKeyPressed(kbEvent1)
        shortcutsManager.onKeyPressed(kbEvent2)
        shortcutsManager.onKeyPressed(kbEvent3)
        shortcutsManager.onKeyPressed(kbEvent5)

        assert(executed == false)

        shortcutsManager.onKeyPressed(kbEvent1)
        shortcutsManager.onKeyPressed(kbEvent2)
        shortcutsManager.onKeyPressed(kbEvent3)

        setTimeout(()=>{
            shortcutsManager.onKeyPressed(kbEvent4)

            assert(executed == false)
        }, 2000)
    })
});
