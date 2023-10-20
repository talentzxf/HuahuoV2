
// Project == Store. (Should be consolidate these two terms??)
import {sceneViewManager} from "./SceneViewManager";
import {SnapshotUtils} from "../Utilities/SnapshotUtils";

class ProjectInfo {
    id: number;
    name:string;
    description: string;
    coverPage: Blob // Image of the cover page.
    inited: boolean = false

    callBackFunctions: Array<Function> = new Array()

    constructor() {
        this.Clear()
    }

    Clear(){
        this.id = -1
        this.name = '';
        this.description = null
        this.coverPage = null
        this.inited = false
    }

    getProjectName(){
        return this.name
    }

    SetProjectName(projectName: string){
        this.name = projectName

        for(let cbFunc of this.callBackFunctions){
            cbFunc()
        }
    }

    Setup(name:string, description:string, coverPageBinary:Blob){
        this.name = name
        this.description = description.trim()
        this.coverPage = coverPageBinary
        this.inited = true

        for(let cbFunc of this.callBackFunctions){
            cbFunc()
        }
    }

    addOnChangedCallback(cbFunc: Function){
        this.callBackFunctions.push(cbFunc)
    }

    updateCoverPage(){
        let sceneView = sceneViewManager.getFocusedSceneView()

        let coverPageBinary = SnapshotUtils.takeSnapshot(sceneView.canvas)
        this.coverPage = coverPageBinary
    }
}

let projectInfo = new ProjectInfo()

export {projectInfo}