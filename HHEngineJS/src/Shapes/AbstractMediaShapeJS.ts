import {BaseShapeJS} from "./BaseShapeJS";
import {getMimeTypeFromDataURI, dataURItoBlob} from "hhcommoncomponents"

// The class is abstract and can't be instantiated.
abstract class AbstractMediaShapeJS extends BaseShapeJS{

    data:string = null
    fileName: string = ""

    loaded: boolean = false

    dirty: boolean = false

    setData(fName, data){
        this.fileName = fName
        this.data = data
        this.dirty = true

        if(this.data)
            this.loaded = true
    }

    awakeFromLoad() {
        let dataLength = this.rawObj.GetDataSize()

        // write the bytes of the string to an ArrayBuffer
        let ab = new ArrayBuffer(dataLength);

        let binaryData:Uint8Array = new Uint8Array(ab)

        for(let idx = 0; idx < dataLength; idx++){
            binaryData[idx] = this.rawObj.GetDataAtIndex(idx);
        }

        this.fileName = this.rawObj.GetFileName()

        // VZ: This code is not working, not sure why.
        // this.rawObj.LoadData(binaryData)
        // console.log("Loaded")

        let mimeType:string = this.rawObj.GetMimeType()
        let blob = new Blob([binaryData], {'type': mimeType})

        let reader = new FileReader()
        reader.readAsDataURL(blob)

        let _this = this
        reader.onload = function(){
            _this.data = reader.result as string
            _this.loaded = true

            _this.onDataLoaded()
        }

        super.awakeFromLoad();
    }

    abstract onDataLoaded();

    store() {
        super.store();

        if(this.data != null && !this.data.startsWith("blob") && this.dirty){
            let binaryData:Uint8Array = dataURItoBlob(this.data)
            this.rawObj.SetFileName(this.fileName) // Should set file name before set data. TODO: Merge these two functions.
            this.rawObj.SetData(binaryData, binaryData.length);

            this.rawObj.SetMimeType(getMimeTypeFromDataURI(this.data))
            this.dirty = false
        }
    }
}

export {AbstractMediaShapeJS}