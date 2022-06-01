function SendMessage(gameObject, func, param)
{
    if (param === undefined)
        Module.ccall("SendMessage", null, ["string", "string"], [gameObject, func]);
    else if (typeof param === "string")
        Module.ccall("SendMessageString", null, ["string", "string", "string"], [gameObject, func, param]);
    else if (typeof param === "number")
        Module.ccall("SendMessageFloat", null, ["string", "string", "number"], [gameObject, func, param]);
    else
        throw "" + param + " is does not have a type which is supported by SendMessage.";
}
Module["SendMessage"] = SendMessage; // to avoid emscripten stripping