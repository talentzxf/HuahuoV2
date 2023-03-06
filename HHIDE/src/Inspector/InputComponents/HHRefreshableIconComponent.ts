import {CustomElement} from "hhcommoncomponents";

@CustomElement({
    selector: "hh-refreshable-icon-component",
    extends: "img"
})
class HHRefreshableIconComponent extends HTMLImageElement implements RefreshableComponent{
    getter: Function

    constructor(getter) {
        super();

        this.getter = getter
    }

    connectedCallback(){
        this.refresh()
    }

    refresh(){
        let [targetComponent, fieldName] = this.getter()

        let binaryResource = targetComponent.rawObj.GetBinaryResource(fieldName)

        let dataLength = binaryResource.GetDataSize()
        let ab = new ArrayBuffer(dataLength)
        let binaryData:Uint8Array = new Uint8Array(ab)
        for(let idx = 0; idx < dataLength; idx++){
            binaryData[idx] = binaryResource.GetDataAtIndex(idx)
        }

        let mimeType: string = binaryResource.GetMimeType()
        let blob = new Blob([binaryData], {'type': mimeType})
        let reader = new FileReader()
        reader.readAsDataURL(blob)

        let _this = this
        reader.onload = function(){
            _this.src = reader.result as string
        }
    }
}

export {HHRefreshableIconComponent}