import {UndoableCommand} from "./UndoManager";

// This command represents a series of commands. They will be executed/undone together.
class CommandArrayCommand extends UndoableCommand {
    commands: Array<UndoableCommand>
    constructor(commands: Array<UndoableCommand>) {
        super();
        this.commands = commands
    }
    GetType(): string {
        return "Array of commands";
    }

    _DoCommand() {
        // Execute from start to end.
        for(let command of this.commands){
            command.DoCommand()
        }
    }

    _UnDoCommand() {
        let reversedCommands = this.commands.reverse()
        for(let command of reversedCommands){
            command.UnDoCommand()
        }
    }
}

export {CommandArrayCommand}