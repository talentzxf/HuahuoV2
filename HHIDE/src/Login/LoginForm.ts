import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-login-form"
})
class LoginForm extends HTMLElement {
    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"
        let container = document.createElement("div")
        container.innerHTML =
            "<style>" +
            "form{" +
            "    width: 400px;\n" +
            "    background-color: rgba(255,255,255,0.13);\n" +
            "    top: 50%;\n" +
            "    left: 50%;\n" +
            "    border-radius: 10px;\n" +
            "    backdrop-filter: blur(10px);\n" +
            "    border: 2px solid rgba(255,255,255,0.1);\n" +
            "    box-shadow: 0 0 40px rgba(8,7,16,0.6);\n" +
            "    padding: 50px 35px;" +
            "}" +
            "form *{\n" +
            "    font-family: 'Poppins',sans-serif;\n" +
            "    letter-spacing: 0.5px;\n" +
            "    outline: none;\n" +
            "    border: none;\n" +
            "}\n" +
            "form h3{\n" +
            "    font-size: 32px;\n" +
            "    font-weight: 500;\n" +
            "    line-height: 42px;\n" +
            "    text-align: center;\n" +
            "}" +
            "" +
            "form input{\n" +
            "    display: block;\n" +
            "    height: 50px;\n" +
            "    width: 100%;\n" +
            "    background-color: rgba(255,255,255,0.07);\n" +
            "    border-radius: 3px;\n" +
            "    padding: 0 10px;\n" +
            "    margin-top: 8px;\n" +
            "    font-size: 14px;\n" +
            "    font-weight: 300;\n" +
            "}" +
            "/* Full-width inputs */\n" +
            "form input[type=text], input[type=password] {" +
            "  width: 100%;" +
            "  padding: 12px 20px;" +
            "  margin: 8px 0;" +
            "  display: inline-block;" +
            "  border: 1px solid #ccc;" +
            "  box-sizing: border-box;" +
            "}" +
            "" +
            "/* Set a style for all buttons */" +
            "form button {" +
            "  background-color: #04AA6D;" +
            "  color: white;" +
            "  padding: 14px 20px;" +
            "  margin: 8px 0;" +
            "  border: none;" +
            "  cursor: pointer;" +
            "  width: 100%;" +
            "}" +
            "</style>"

        container.innerHTML+=
            "   <div>" +
            "       <img></img>" +
            "   </div>" +
            "   <form>" +
            "       <h3>Login Here</h3>" +
            "       <label for='uname'><b>Username</b></label>" +
            "       <input type='text' placeholder='Enter Username' name='username'> " +
            "       <label for='pwd'><b>Password</b></label>" +
            "       <input type='password' placeholder='Enter Password' name='password'> " +
            "       <button type='submit'>Login</button>" +
            "   </form>"

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