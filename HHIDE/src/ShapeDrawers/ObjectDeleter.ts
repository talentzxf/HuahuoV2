import {Logger} from "hhcommoncomponents"
import {BaseShapeJS,huahuoEngine} from "hhenginejs";

class ObjectDeleter{
    deleteSegment(segment: paper.Segment){
        let baseShape = segment.path.data.meta
        baseShape.removeSegment(segment)
    }

    deleteShape(shape: BaseShapeJS){
        shape.remove(); // Remove this object
    }
}

let objectDeleter = new ObjectDeleter()
export {objectDeleter}