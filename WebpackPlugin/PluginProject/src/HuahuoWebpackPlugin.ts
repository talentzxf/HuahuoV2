import {Compiler} from "webpack"

class HuahuoWebpackPlugin {

    constructor(private options: any) {
    }

    apply(compiler: Compiler){
        compiler.hooks.emit.tapAsync("HuahuoWebpackPlugin", (compilation, callback)=>{
            console.log("Compiled file:", Object.keys(compilation.assets))

            callback();
        })
    }
}

export {HuahuoWebpackPlugin}