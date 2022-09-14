
// Project == Store. (Should be consolidate these two terms??)
class StoreInfo{
    name:string;
    description: string;
    coverPage: Uint8Array // Image of the cover page.
    inited: boolean = false

    Clear(){
        this.name = "";
        this.description = ""
        this.coverPage = new Uint8Array()
        this.inited = false
    }

    Setup(name, description, frameId:number = 0, canvas: HTMLCanvasElement){
        this.name = name
        this.description = description

        // TODO: Take photo of the selected frameId and render in the specified canvas.

    }
}

let storeInfo = new StoreInfo()

export {storeInfo}