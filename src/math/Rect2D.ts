import {Vector2D} from "./Vector2D";
class Rect2D{
    private leftUp: Vector2D = new Vector2D()
    private rightDown: Vector2D = new Vector2D()

    public static fromDomRect(domRect:DOMRect){
        return new Rect2D(domRect.x, domRect.y, domRect.x + domRect.width, domRect.y +domRect.height)
    }

    public constructor(x1: number, y1: number, x2: number, y2: number) {
        this.leftUp.X = x1
        this.leftUp.Y = y1
        this.rightDown.X = x2
        this.rightDown.Y = y2
    }

    public in(p: Vector2D):boolean{
        if(p.X < this.leftUp.X || p.X > this.rightDown.X) return false
        if(p.Y < this.leftUp.Y || p.Y > this.rightDown.Y) return false

        return true
    }

    public overlap(otherRect: Rect2D): boolean{
        if(this.in(otherRect.leftUp) || this.in(otherRect.rightDown)){
            return true
        }

        if(otherRect.in(this.leftUp) || otherRect.in(this.rightDown))
        {
            return true
        }

        return false
    }

    public toString(): string{
        return "(" + this.leftUp.X + "," + this.leftUp.Y + ")->(" + this.rightDown.X + "," + this.rightDown.Y + ")"
    }
}

export {Rect2D}