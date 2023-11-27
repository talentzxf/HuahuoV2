import {Compiler} from "webpack"
import {Chunk} from "webpack";

class HuahuoWebpackPlugin {

    chunkVersions = new Map<string, string>()

    apply(compiler: Compiler){
        compiler.hooks.emit.tapAsync("HuahuoWebpackPlugin", (compilation, callback)=>{
            compilation.modules.forEach((module)=>{
                // @ts-ignore
                if(module.resource && module._source){
                    // @ts-ignore
                    console.log("Module name:", module.resource)
                    // @ts-ignore
                    console.log("Module source", module._source._value.toString())
                }
            })

            callback()
        })
    }
}

export {HuahuoWebpackPlugin}