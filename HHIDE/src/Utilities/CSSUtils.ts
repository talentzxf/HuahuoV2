class CSSUtils {
    static css2obj(css){
        if(!css)
            return {}

        const r = /(?<=^|;)\s*([^:]+)\s*:\s*([^;]+)\s*/g, o = {};
        css.replace(r, (m,p,v) => o[p] = v);
        return o;
    }

    static formStyle = "<style>" +
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
}

export {CSSUtils}