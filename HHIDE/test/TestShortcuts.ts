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
            code: "KeyZ"
        })
        shortcutsManager.onKeyDown(kbEvent)

        assert(hasUndone)

        hasUndone = false
        let hasDone = false
        let kbEvent2 = new KeyboardEvent("keydown", {
            ctrlKey: true,
            shiftKey: true,
            code: "KeyZ"
        })
        shortcutsManager.registerShortcutHandler(ShortcutEventNames.REDO, ()=>{
            hasDone = true
        })
        shortcutsManager.onKeyDown(kbEvent2)
        assert(hasUndone == false && hasDone)
    })

    it("Test Custom events", ()=>{
        shortcutsManager.init()

        let executed = false
        shortcutsManager.registerKeyCodeSeries([{code:"KeyA"}, {code:"KeyB"}, {code:"KeyC"},{code:"KeyD"}], ()=>{
            executed = true
        })

        let kbEvent1 = new KeyboardEvent("keydown", {
            code: "KeyA"
        })
        let kbEvent2 = new KeyboardEvent("keydown", {
            code: "KeyB"
        })
        let kbEvent3 = new KeyboardEvent("keydown", {
            code: "KeyC"
        })
        let kbEvent4 = new KeyboardEvent("keydown", {
            code: "KeyD"
        })

        shortcutsManager.onKeyDown(kbEvent1)
        shortcutsManager.onKeyDown(kbEvent2)
        shortcutsManager.onKeyDown(kbEvent3)
        shortcutsManager.onKeyDown(kbEvent4)

        assert(executed)

        executed = false
        let kbEvent5 = new KeyboardEvent("keydown", {
            key: "e"
        })

        shortcutsManager.onKeyDown(kbEvent1)
        shortcutsManager.onKeyDown(kbEvent2)
        shortcutsManager.onKeyDown(kbEvent3)
        shortcutsManager.onKeyDown(kbEvent5)

        assert(executed == false)

        shortcutsManager.onKeyDown(kbEvent1)
        shortcutsManager.onKeyDown(kbEvent2)
        shortcutsManager.onKeyDown(kbEvent3)

        setTimeout(()=>{
            shortcutsManager.onKeyDown(kbEvent4)

            assert(executed == false)
        }, 2000)
    })
});
