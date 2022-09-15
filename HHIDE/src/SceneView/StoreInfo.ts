
// Project == Store. (Should be consolidate these two terms??)
class StoreInfo{
    name:string;
    description: string;
    coverPage: Blob // Image of the cover page.
    inited: boolean = false

    Clear(){
        this.name = "";
        this.description = ""
        this.coverPage = null
        this.inited = false
    }

    Setup(name:string, description:string, coverPageBinary:Blob){
        this.name = name
        this.description = description
        this.coverPage = coverPageBinary
    }
}

let storeInfo = new StoreInfo()

export {storeInfo}