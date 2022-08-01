import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-login-form"
})
class LoginForm extends HTMLElement {
    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        let container = document.createElement("form")
        container.innerHTML = "" +
            "<style>" +
            "form{" +
            "   border: 3px solid #f1f1f1" +
            "}" +
            "" +
            "/* Full-width inputs */\n" +
            "input[type=text], input[type=password] {" +
            "  width: 100%;" +
            "  padding: 12px 20px;" +
            "  margin: 8px 0;" +
            "  display: inline-block;" +
            "  border: 1px solid #ccc;" +
            "  box-sizing: border-box;" +
            "}" +
            "" +
            "/* Set a style for all buttons */" +
            "button {" +
            "  background-color: #04AA6D;" +
            "  color: white;" +
            "  padding: 14px 20px;" +
            "  margin: 8px 0;" +
            "  border: none;" +
            "  cursor: pointer;" +
            "  width: 100%;" +
            "}" +
            "</style>" +
            "<div class=container>" +
            "   <label for='uname'><b>Username</b></label>" +
            "   <input type='text' placeholder='Enter Username' name='username'> " +
            "   <label for='pwd'><b>Password</b></label>" +
            "   <input type='password' placeholder='Enter Password' name='password'> " +
            "   <button type='submit'>Login</button>" +
            "</div>"

        this.appendChild(container)
    }
}

let loginForm = null;

function openLoginForm() {
    if(loginForm == null){
        loginForm = document.createElement("hh-login-form")
        document.body.appendChild(loginForm)
    }


}
export {openLoginForm}