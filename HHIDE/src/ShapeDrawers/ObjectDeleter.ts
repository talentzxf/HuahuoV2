import {BaseShapeJS} from "hhenginejs";

class ObjectDeleter{
    deleteSegment(segment: paper.Segment){

    }

    deleteShape(shape: BaseShapeJS){
        shape.remove(); // Remove this object
    }
}

let objectDeleter = new ObjectDeleter()
export {objectDeleter}