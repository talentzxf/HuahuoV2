import {HHTitle} from "../HHTitle"
class OccupiedTitleManager{
    private static Instance:OccupiedTitleManager;

    private mOccupiedTitle:HHTitle;
    private mOccupiedGroupHolder:HTMLElement;


    constructor() {
        this.mOccupiedTitle = null
        this.mOccupiedGroupHolder = null
    }

    public static getInstance():OccupiedTitleManager{
        if(OccupiedTitleManager.Instance == null){
            OccupiedTitleManager.Instance = new OccupiedTitleManager()
        }

        return OccupiedTitleManager.Instance
    }

    public setCandidate(inCandidateEle:HHTitle, inCandidateGroupHolder:HTMLElement, width:number): void{
        this.Clear()
        this.mOccupiedTitle = inCandidateEle
        this.mOccupiedGroupHolder = inCandidateGroupHolder
        if(width != null){
            this.mOccupiedTitle.setMarginLeft(width)
        }
    }

    public isCurrentGroupHolder(inGroupHolder: HTMLElement){
        return this.mOccupiedGroupHolder === inGroupHolder
    }

    private Clear():void {
        if(this.mOccupiedTitle != null){
            this.mOccupiedTitle = null
            this.mOccupiedGroupHolder = null
        }
    }
}

export {OccupiedTitleManager}