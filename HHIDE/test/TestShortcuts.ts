import {ShortcutEventNames, shortcutsManager} from "../src/Shortcuts/ShortcutsManager";
import "mocha"
import {assert} from "chai"

describe('TestShortcuts manager', ()=> {
    it("test", ()=>{
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

        assert(hasUndone == false)
    })
});
