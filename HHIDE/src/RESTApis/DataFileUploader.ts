import {userInfo} from "../Login/UserInfo";
import {openLoginForm} from "../Login/LoginForm";

class DataFileUploader{
    upload(){
        if(!userInfo.isLoggedIn){
            // Prompt login window
            let loginForm = openLoginForm()
        }
    }
}

let dataFileUploader = new DataFileUploader()
export {dataFileUploader}