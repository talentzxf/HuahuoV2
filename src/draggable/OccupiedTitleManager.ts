import {HHTitle} from "../HHTitle"
import {HHPanel} from "../HHPanel";
import {DomHelper} from "../DomHelper";

class OccupiedTitleManager {
    private static Instance: OccupiedTitleManager;

    private mOccupiedTitle: HHTitle;
    private mTargetPanel: HHPanel;
    private mIsRightMost: Boolean = false;

    constructor() {
        this.mOccupiedTitle = null
        this.mTargetPanel = null
    }

    public static getInstance(): OccupiedTitleManager {
        if (OccupiedTitleManager.Instance == null) {
            OccupiedTitleManager.Instance = new OccupiedTitleManager()
        }

        return OccupiedTitleManager.Instance
    }

    public setCandidate(inCandidateEle: HHTitle, targetPanel: HHPanel, width: number): void {
        this.Clear()
        this.mIsRightMost = false
        this.mOccupiedTitle = inCandidateEle
        this.mTargetPanel = targetPanel
        if (width != null) {
            this.mOccupiedTitle.setMarginLeft(width)
        }
    }

    public isCurrentGroupHolder(inGroupHolder: HTMLElement) {
        return this.mTargetPanel === inGroupHolder
    }

    public Clear(): void {
        if (this.mOccupiedTitle != null) {
            this.mOccupiedTitle.setMarginLeft(0)
            this.mOccupiedTitle = null
            this.mTargetPanel = null
            this.mIsRightMost = false
        }
    }

    setIsRightMost(targetPanel: HHPanel) {
        this.Clear()
        this.mIsRightMost = true
        this.mTargetPanel = targetPanel;
        this.mOccupiedTitle = null;
    }

    adjustTabIndices(panel: HHPanel, start: number, end: number, amount: number, skipFunc: Function = null) {
        console.log("Adjusting:" + start + "," + end + " amount:" + amount)
        let tobeAdjustedTitles: HHTitle[] = panel.getTitles(start, end)
        tobeAdjustedTitles.forEach(title => {
            if (skipFunc == null || !skipFunc(title))
                title.setTabIndex(title.tabIndex + amount)
        })
    }

    dropTitle(title: HHTitle) {
        let oldPanel = title.getParentPanel();
        let oldIndex: number = title.tabIndex;
        let newIndex: number = -1

        title.setStylePosition("static")

        if (this.mIsRightMost) { // Put the title as the last one of the title group
            newIndex = this.mTargetPanel.getTitleCount()
            if (this.mTargetPanel == oldPanel) {
                newIndex--;
            }
        } else {
            // If it's the left to right case, insert before the candidate.
            // Or else, take the place of the candidate.
            if (this.mTargetPanel == oldPanel && this.mOccupiedTitle.tabIndex > oldIndex) {
                newIndex = this.mOccupiedTitle.tabIndex - 1;
            } else {
                newIndex = this.mOccupiedTitle.tabIndex
            }
        }

        title.setTabIndex(newIndex);

        if (this.mTargetPanel != oldPanel) {
            this.mTargetPanel.addChild(title);
        }

        function skipFunc(inTitle: HHTitle) {
            return inTitle == title
        }

        if (this.mTargetPanel == oldPanel) { // Internal adjust
            if (oldIndex == newIndex) {
                this.Clear()
                return;
            }

            // true -- left to right
            // false -- right to left
            let direction: boolean = oldIndex < newIndex;
            if (direction) {
                this.adjustTabIndices(this.mTargetPanel, oldIndex + 1, newIndex, -1, skipFunc)
            } else {
                this.adjustTabIndices(this.mTargetPanel, newIndex, oldIndex, 1, skipFunc)
            }
        } else {
            let totalTitleCount = this.mTargetPanel.getTitleCount();
            this.adjustTabIndices(this.mTargetPanel, newIndex, totalTitleCount - 1, 1, skipFunc)
            this.adjustTabIndices(oldPanel, oldIndex + 1, oldPanel.getTitleCount(), -1)
            if (oldPanel.getTitleCount() == 0) {
                let oldParent = oldPanel.parentElement
                this.removeElementWithSplitter(oldPanel)
                this.RecursivelyRemoveEmptyParents(oldParent)
            } else if (title.getAttribute('selected') == 'true') {
                oldPanel.selectTab(0)
            }
        }

        this.mTargetPanel.renderTitles()
        this.mTargetPanel.selectTab(newIndex)

        this.Clear()
    }

    RecursivelyRemoveEmptyParents(ele: HTMLElement){
        let panel = ele.querySelector("hh-panel")
        while(panel == null && ele.parentElement != null){
            let parentElement = ele.parentElement

            if(ele.nodeName.toLowerCase() == "hh-container"){
                this.removeElementWithSplitter(ele)
            }
            panel = parentElement.querySelector('hh-panel')
        }
    }

    removeElementWithSplitter(ele: HTMLElement){

        let oldParent = ele.parentElement
        // Delete the panel and it's next splitter
        let nextSplitter = DomHelper.getNextSiblingElementByName(ele, "hh-splitter")
        // If this is the next, delete it's previous splitter also.
        let nextPanel = DomHelper.getNextSiblingElementByName(ele, ele.nodeName.toLowerCase())
        let prevSplitter = null
        if(!nextPanel){
            prevSplitter = DomHelper.getPrevSiblingElementByName(ele, "hh-splitter")
        }

        oldParent.removeChild(ele)
        if(nextSplitter)
            oldParent.removeChild(nextSplitter)
        if(prevSplitter)
            oldParent.removeChild(prevSplitter)

    }
}

export {OccupiedTitleManager}