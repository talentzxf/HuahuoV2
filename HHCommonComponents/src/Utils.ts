import * as paper from "paper"

function pointsNear(p1:paper.Point, p2:paper.Point, margin:number){
    return p1.getDistance(p2) < margin
}

function relaxRectangle(rectangle, margin) {
    let retRectangle = rectangle.clone()
    retRectangle.x -= margin
    retRectangle.y -= margin
    retRectangle.width += 2 * margin
    retRectangle.height += 2 * margin

    return retRectangle
}

function getMimeTypeFromDataURI(dataURI): string{
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
export {pointsNear,relaxRectangle, getMimeTypeFromDataURI, dataURItoBlob}