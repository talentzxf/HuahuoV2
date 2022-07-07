// This piece of code is almost the same as LineDrawer, maybe we should extract a common base class??
import {BaseShapeDrawer} from "./BaseShapeDrawer";

class TextDrawer extends BaseShapeDrawer{
    name = "Text"
    imgClass = "far fa-edit"

}

export {TextDrawer}