class Vector2D{
    public X:number
    public Y:number

    public constructor(X?:number,Y?:number) {
        this.X = X || 0
        this.Y = Y || 0
    }

    equals(otherX: number, otherY: number){
        if(this.X === otherX && this.Y === otherY){
            return true;
        }
        return false;
    }
}

export {Vector2D}