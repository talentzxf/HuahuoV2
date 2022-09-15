class SnapshotUtils{
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