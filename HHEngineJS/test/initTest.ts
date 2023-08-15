import {huahuoEngine} from "../src";

function initTest(){
    huahuoEngine.ExecuteAfterInited(()=>{
        console.log("Engine inited!")
    })
}

export {initTest}