
class Vector2{
    x: number
    y: number

    constructor(x = 0.0,y = 0.0) {
        this.x = x
        this.y = y
    }

    distance(p:Vector2){
        return Math.sqrt( (this.x - p.x ) ** 2 + (this.y - p.y ) ** 2)
    }

    subtract(p:Vector2){
        return new Vector2(this.x - p.x, this.y - p.y)
    }

    add(p:Vector2){
        return new Vector2(this.x + p.x, this.y + p.y)
    }

    length(){
        return Math.sqrt(this.x**2 + this.y**2)
    }
}

export {Vector2}