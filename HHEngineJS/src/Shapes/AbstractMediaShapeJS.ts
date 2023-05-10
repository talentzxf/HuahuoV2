import {BaseShapeJS} from "./BaseShapeJS";
import {PropertyType} from "hhcommoncomponents"

// The class is abstract and can't be instantiated.
abstract class AbstractMediaShapeJS extends BaseShapeJS {
    resourceMD5: string

    data

    isLoaded() {
        return this.data != null
    }

    setResourceByMD5(md5) {
        if (this.resourceMD5 != md5) {
            this.rawObj.SetResourceByMD5(md5)
            return this.loadDataFromCpp()
        }
    }

    loadDataFromCpp() {
        let dataLength = this.rawObj.GetDataSize()

        // write the bytes of the string to an ArrayBuffer
        let ab = new ArrayBuffer(dataLength);

        let binaryData: Uint8Array = new Uint8Array(ab)

        for (let idx = 0; idx < dataLength; idx++) {
            binaryData[idx] = this.rawObj.GetDataAtIndex(idx);
        }

        let mimeType: string = this.rawObj.GetMimeType()
        let blob = new Blob([binaryData], {'type': mimeType})

        let reader = new FileReader()
        reader.readAsDataURL(blob)

        let resolveFunction
        let loadImagePromise = new Promise((resolve, reject) => {
            resolveFunction = resolve
        })

        let _this = this
        reader.onload = function () {
            _this.data = reader.result as string
            _this.resourceMD5 = _this.rawObj.GetResourceMD5()
            _this.onDataLoaded()

            resolveFunction()
        }

        return loadImagePromise
    }

    update(force: boolean = false) {
        if (this.isLoaded()) {
            super.update(force);
        }
    }

    awakeFromLoad() {
        this.loadDataFromCpp().then(() => {
            super.awakeFromLoad();
        })
    }

    appendProperties() {
        let _this = this
        this.propertySheet.addProperty({
            key: "inspector.media.filename",
            type: PropertyType.STRING,
            getter: () => {
                return _this.rawObj.GetFileName()
            }
        })
    }

    abstract onDataLoaded();
}

export {AbstractMediaShapeJS}