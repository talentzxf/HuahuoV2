import {AbstractComponent, Component} from "../AbstractComponent";

@Component({compatibleShapes: ["ElementShapeJS"], maxCount: 1})
class RigidBody extends AbstractComponent{
    constructor(rawObj?, isMirage = false) {
        super(rawObj, isMirage);

        
    }
}

export {RigidBody}