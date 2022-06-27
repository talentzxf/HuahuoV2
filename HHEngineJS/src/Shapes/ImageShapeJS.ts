import {BaseShapeJS, shapeFactory} from "./BaseShapeJS";

function getMimeType(dataURI): string{
    // separate out the mime component
    let mimeString = dataURI.split(',')[0].split(':')[1].split(';')[0]
    return mimeString
}

function dataURItoBlob(dataURI): Uint8Array{
    // convert base64 to raw binary data held in a string
    // doesn't handle URLEncoded DataURIs - see SO answer #6850276 for code that does this
    let byteString = atob(dataURI.split(',')[1]);

    // write the bytes of the string to an ArrayBuffer
    let ab = new ArrayBuffer(byteString.length);

    // create a view into the buffer
    let ia = new Uint8Array(ab);

    // set the bytes of the buffer to the correct values
    for (let i = 0; i < byteString.length; i++) {
        ia[i] = byteString.charCodeAt(i);
    }
    return ia;
}

let shapeName = "ImageShape"
class ImageShapeJS extends BaseShapeJS{

    static createImageShape(rawObj){
        return new ImageShapeJS(rawObj);
    }

    imgData:string = null
    fileName: string = ""

    loaded: boolean = false

    dirty: boolean = false

    setImageData(fName, data){
        this.fileName = fName
        this.imgData = data
        this.dirty = true
    }

    getShapeName(): string {
        return shapeName
    }

    awakeFromLoad() {
        let dataLength = this.rawObj.GetImageDataSize()

        // write the bytes of the string to an ArrayBuffer
        let ab = new ArrayBuffer(dataLength);

        let binaryData:Uint8Array = new Uint8Array(ab)

        for(let idx = 0; idx < dataLength; idx++){
            binaryData[idx] = this.rawObj.GetImageDataAtIndex(idx);
        }
        // this.rawObj.LoadImageData(binaryData)
        // console.log("Loaded")

        let mimeType:string = this.rawObj.GetImageMimeType()
        let blob = new Blob([binaryData], {'type': mimeType})
        this.imgData = URL.createObjectURL(blob)

        super.awakeFromLoad();
    }

    // TODO: Create GIF shape later.
    createShape() {
        let _this = this
        let _paper:any = this.getPaperJs()
        let tempShape = new _paper.Raster()
        tempShape.data.meta = this
        tempShape.position = _paper.view.center
        tempShape.source = this.imgData

        tempShape.onLoad = function(){
            _this.loaded = true

            _this.dirty = false
        }

        this.paperItem = tempShape
    }

    store() {
        super.store();

        if(!this.imgData.startsWith("blob") && this.dirty){
            let binaryData:Uint8Array = dataURItoBlob(this.imgData)
            this.rawObj.SetImageData(binaryData, binaryData.length);
            this.rawObj.SetImageMimeType(getMimeType(this.imgData))
        }
    }
}

shapeFactory.RegisterClass(shapeName, ImageShapeJS.createImageShape)

export {ImageShapeJS}