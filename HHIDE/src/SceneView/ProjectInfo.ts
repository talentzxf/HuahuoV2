
// Project == Store. (Should be consolidate these two terms??)
import {sceneViewManager} from "./SceneViewManager";
import {SnapshotUtils} from "../Utilities/SnapshotUtils";
import {EventNames, IDEEventBus} from "../Events/GlobalEvents";

class ProjectInfo {
    id: number;
    name:string;
    description: string;
    coverPage: Blob // Image of the cover page.
    inited: boolean = false

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

        IDEEventBus.getInstance().emit(EventNames.PROJECTINFOUPDATED)
    }

    Setup(name:string, description:string, coverPageBinary:Blob){
        this.name = name
        this.description = description.trim()
        this.coverPage = coverPageBinary
        this.inited = true

        IDEEventBus.getInstance().emit(EventNames.PROJECTINFOUPDATED)
    }

    updateCoverPage(){
        let sceneView = sceneViewManager.getFocusedSceneView()

        let coverPageBinary = SnapshotUtils.takeSnapshot(sceneView.canvas)
        this.coverPage = coverPageBinary
    }
}

let projectInfo = new ProjectInfo()

export {projectInfo}