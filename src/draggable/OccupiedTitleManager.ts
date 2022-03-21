import {HHTitle} from "../HHTitle"
import {HHPanel} from "../HHPanel";
import {DomHelper} from "../utilities/DomHelper";
import {ShadowPanelManager} from "./ShadowPanelManager";
import {HHContainer} from "../HHContainer";

enum SplitPanelDir {
    UP,
    DOWN,
    LEFT,
    RIGHT
}

enum DropTitleType {
    NONE,
    TITLEBAR,
    RIGHTMOST,
    SPLITPANEL,
}

function TypeIsColumn(type: SplitPanelDir): Boolean {
    return type == SplitPanelDir.UP || type == SplitPanelDir.DOWN
}

function TypeIsFirst(type: SplitPanelDir): Boolean {
    return type == SplitPanelDir.LEFT || type == SplitPanelDir.UP
}

function ColumnRowString(type: SplitPanelDir): string {
    return TypeIsColumn(type)?"column":"row"
}

class OccupiedTitleManager {
    private static Instance: OccupiedTitleManager;

    private mDropType: DropTitleType = DropTitleType.NONE;
    private mOccupiedTitle: HHTitle;
    private mTargetPanel: HHPanel;
    private mSplitPanelDir: SplitPanelDir;

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
        this.mDropType = DropTitleType.TITLEBAR
        this.mOccupiedTitle = inCandidateEle
        this.mTargetPanel = targetPanel
        if (width != null) {
            this.mOccupiedTitle.setMarginLeft(width)
        }
    }

    public setIsRightMost(targetPanel: HHPanel) {
        this.Clear()
        this.mDropType = DropTitleType.RIGHTMOST
        this.mTargetPanel = targetPanel;
        this.mOccupiedTitle = null;
    }

    public setShadowCandidate(targetPanel: HHPanel, splitPanelDir: SplitPanelDir) {
        this.Clear()
        this.mTargetPanel = targetPanel
        this.mDropType = DropTitleType.SPLITPANEL
        this.mSplitPanelDir = splitPanelDir
    }

    public isCurrentGroupHolder(inGroupHolder: HTMLElement) {
        return this.mTargetPanel === inGroupHolder
    }

    public Clear(): void {
        if (this.mOccupiedTitle != null) {
            this.mOccupiedTitle.setMarginLeft(0)
            this.mOccupiedTitle = null
            this.mTargetPanel = null
            this.mDropType = DropTitleType.NONE
        }
    }

    adjustTabIndices(panel: HHPanel, start: number, end: number, amount: number, skipFunc: Function = null) {
        console.log("Adjusting:" + start + "," + end + " amount:" + amount)
        let tobeAdjustedTitles: HHTitle[] = panel.getTitles(start, end)
        tobeAdjustedTitles.forEach(title => {
            if (skipFunc == null || !skipFunc(title))
                title.setTabIndex(title.tabIndex + amount)
        })
    }

    createContainer(direction: string, width: string, height: string):HHContainer{
        // let newParentContainer = document.createElement('hh-container') as HHContainer
        // newParentContainer.id = "newParentContainer"
        // newParentContainer.setAttribute("direction", TypeIsColumn(this.mSplitPanelDir) ? "column" : "row")
        // newParentContainer.style.width = this.mTargetPanel.style.width
        // newParentContainer.style.height = this.mTargetPanel.style.height
        // return newParentContainer

        let newParentContainer = document.createElement('hh-container') as HHContainer
        newParentContainer.id = "newParentContainer"
        newParentContainer.setAttribute("direction", direction)
        newParentContainer.style.width = width
        newParentContainer.style.height = height
        return newParentContainer
    }

    splitPanel(title: HHTitle) {
        let parentContainer: HTMLElement = this.mTargetPanel.parentElement
        let grandParentElement: HTMLElement = parentContainer.parentElement
        let panelContainer = this.createContainer("column", "100%", "100%")
        let newPanel = document.createElement('hh-panel') as HHPanel
        newPanel.id = "newPanel"
        newPanel.style.width = "100%"
        newPanel.style.height = "100%"
        panelContainer.append(newPanel)

        let newParentContainer = this.createContainer(ColumnRowString(this.mSplitPanelDir), parentContainer.style.width, parentContainer.style.height)
        grandParentElement.insertBefore(newParentContainer, parentContainer)

        let newSplitter = document.createElement("hh-splitter")
        newSplitter.setAttribute("direction", TypeIsColumn(this.mSplitPanelDir) ? "column" : "row")

        if (TypeIsFirst(this.mSplitPanelDir)) {
            newParentContainer.append(panelContainer)
            newParentContainer.append(newSplitter)
            newParentContainer.append(parentContainer)
        } else {
            newParentContainer.append(parentContainer)
            newParentContainer.append(newSplitter)
            newParentContainer.append(panelContainer)
        }



        title.setParentPanel(newPanel)

        this.mTargetPanel = title.getParentPanel()
    }

    dropTitle(title: HHTitle) {
        ShadowPanelManager.getInstance().hideShadowPanel()
        let oldPanel = title.getParentPanel();
        let oldIndex: number = title.tabIndex;
        let newIndex: number = -1

        title.setStylePosition("static")

        // Should we use enum-function map to avoid this ugly switch-case ??
        switch (this.mDropType) {
            case DropTitleType.RIGHTMOST:
                newIndex = this.mTargetPanel.getTitleCount()
                if (this.mTargetPanel == oldPanel) {
                    newIndex--;
                }
                break;
            case DropTitleType.TITLEBAR:
                // If it's the left to right case, insert before the candidate.
                // Or else, take the place of the candidate.
                if (this.mTargetPanel == oldPanel && this.mOccupiedTitle.tabIndex > oldIndex) {
                    newIndex = this.mOccupiedTitle.tabIndex - 1;
                } else {
                    newIndex = this.mOccupiedTitle.tabIndex
                }
                break;
            case DropTitleType.SPLITPANEL:
                this.splitPanel(title)
                newIndex = 0 // As this is a new container, this is always the first title in the panel
            default:
                break;
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

    RecursivelyRemoveEmptyParents(ele: HTMLElement) {
        let panel = ele.querySelector("hh-panel")
        while (panel == null && ele.parentElement != null) {
            let parentElement = ele.parentElement

            if (ele.nodeName.toLowerCase() == "hh-container") {
                this.removeElementWithSplitter(ele)
            }
            panel = parentElement.querySelector('hh-panel')
        }
    }

    removeElementWithSplitter(ele: HTMLElement) {

        let oldParent = ele.parentElement
        // Delete the panel and it's next splitter
        let nextSplitter = DomHelper.getNextSiblingElementByName(ele, ["hh-splitter"])
        // If this is the next, delete it's previous splitter also.
        let nextPanel = DomHelper.getNextSiblingElementByName(ele, [ele.nodeName.toLowerCase()])
        let prevSplitter = null
        if (!nextPanel) {
            prevSplitter = DomHelper.getPrevSiblingElementByName(ele, ["hh-splitter"])
        }

        oldParent.removeChild(ele)
        if (nextSplitter)
            oldParent.removeChild(nextSplitter)
        if (prevSplitter)
            oldParent.removeChild(prevSplitter)

    }
}

export {OccupiedTitleManager, SplitPanelDir}