class Logger{
    static debug(msg){
        console.log("DEBUG:" + msg)
    }

    static info(msg){
        console.log("INFO:" + msg)
    }

    static warn(msg){
        console.log("WARNING:" + msg)
    }

    static error(msg){
        console.log("ERROR:" + msg)
    }
}
export {Logger}