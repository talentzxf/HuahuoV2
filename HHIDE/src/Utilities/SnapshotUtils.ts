import {huahuoEngine, renderEngine2D, Player} from "hhenginejs";

let hiddenCanvas = document.createElement("canvas")
let previewPlayer = new Player()

class SnapshotUtils{

    // TODO: Merge all takesnapshot functions here. Avoid code duplication.
    static async takeSnapShotForStore(storeId): Promise<Blob>{
        // document.body.appendChild(hiddenCanvas)
        // hiddenCanvas.style.position = "absolute"
        // hiddenCanvas.style.left = "0px"
        // hiddenCanvas.style.top = "0px"
        // hiddenCanvas.style.border = "1px solid black"

        let previousCanvas = renderEngine2D.getDefaultCanvas()
        renderEngine2D.init(hiddenCanvas, true)

        let [initW, initH] = renderEngine2D.getInitCanvasWH()
        let [contentW, contentH] = renderEngine2D.getContentWH(initW, initH)
        if(initW > 0){
            renderEngine2D.resize(hiddenCanvas, contentW, contentH)
        }

        let prevStore = huahuoEngine.GetCurrentStoreId()
        try{
            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(storeId)

            let currentLayer = huahuoEngine.GetCurrentLayer()
            let currentFrameId = currentLayer.GetCurrentFrame()
            renderEngine2D.setDefaultCanvas(hiddenCanvas)

            previewPlayer.storeId = storeId
            previewPlayer.loadShapesFromStore()
            previewPlayer.setFrameId(currentFrameId)
        }finally {
            if (previousCanvas)
                renderEngine2D.setDefaultCanvas(previousCanvas)

            huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(prevStore)
        }

        return new Promise<Blob>((resolve, reject)=>{

            setTimeout(function(){
                let resultBlob = SnapshotUtils.takeSnapshot(hiddenCanvas)
                resolve(resultBlob)
            }, 10)
        })
    }
    static takeSnapshot(canvas:HTMLCanvasElement): Blob{
        let BASE64_MARKER = ";base64,";
        let dataURL = canvas.toDataURL()
        if(dataURL.indexOf(BASE64_MARKER) == -1){
            let parts = dataURL.split(",");
            let contentType = parts[0].split(":")[1];
            let raw = decodeURIComponent(parts[1])
            return new Blob([raw], {type: contentType})
        }

        let parts = dataURL.split(BASE64_MARKER)
        let contentType = parts[0].split(":")[1];
        let raw = window.atob(parts[1])
        let rawLength = raw.length

        let uInt8Array = new Uint8Array(rawLength)
        for(let i = 0; i < rawLength; i++){
            uInt8Array[i] = raw.charCodeAt(i)
        }

        return new Blob([uInt8Array], {type: contentType})
    }
}

export {SnapshotUtils}