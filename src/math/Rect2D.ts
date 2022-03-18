import {Vector2D} from "./Vector2D";
class Rect2D{
    private leftUp: Vector2D = new Vector2D()
    private rightDown: Vector2D = new Vector2D()

    get height():number{
        return this.rightDown.Y - this.leftUp.Y
    }

    get width():number{
        return this.rightDown.X - this.leftUp.X
    }

    public static fromDomRect(domRect:DOMRect){
        return new Rect2D(domRect.x, domRect.y, domRect.x + domRect.width, domRect.y +domRect.height)
    }

    public constructor(x1: number, y1: number, x2: number, y2: number) {
        this.leftUp.X = x1
        this.leftUp.Y = y1
        this.rightDown.X = x2
        this.rightDown.Y = y2
    }

    public getRightDown(){
        return this.rightDown
    }

    public getLeftUp(){
        this.leftUp
    }

    public in(p: Vector2D):boolean{
        if(p.X < this.leftUp.X || p.X > this.rightDown.X) return false
        if(p.Y < this.leftUp.Y || p.Y > this.rightDown.Y) return false

        return true
    }

    public overlap(otherRect: Rect2D): boolean{

        // Either one of the rectanges is a line
        if(this.leftUp.Y == this.rightDown.Y || this.leftUp.X == this.rightDown.X ||
            otherRect.leftUp.X == otherRect.rightDown.X || otherRect.leftUp.Y == otherRect.rightDown.Y)
            return false;

        // Either one is in the left of another one.
        if(this.leftUp.X > otherRect.rightDown.X || this.rightDown.X < otherRect.leftUp.X){
            return false
        }

        // Either one is on top of another
        if(this.leftUp.Y > otherRect.rightDown.Y || this.rightDown.Y < otherRect.leftUp.Y)
        {
            return false
        }

        return true
    }

    public toString(): string{
        return "(" + this.leftUp.X + "," + this.leftUp.Y + ")->(" + this.rightDown.X + "," + this.rightDown.Y + ")"
    }
}

export {Rect2D}