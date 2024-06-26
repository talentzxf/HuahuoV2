class ColorStop{
    identifier
    value
    r
    g
    b
    a
    constructor(rawObj?) {
        if(rawObj){
            this.identifier = rawObj.GetIdentifier()
            this.value = rawObj.GetValue();
            let color = rawObj.GetColor();
            this.r = color.r;
            this.g = color.g;
            this.b = color.b;
            this.a = color.a;
        }
    }
}

export {ColorStop}