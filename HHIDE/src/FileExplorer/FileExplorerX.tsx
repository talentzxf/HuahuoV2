import * as React from "react"
import {HierarchyItem} from "../Hierarchy/HierarchyX";
import {huahuoEngine} from "hhenginejs";
import {IsValidWrappedObject} from "hhcommoncomponents";

class FileExplorerX extends React.Component<any, any> {
    regSetter(uuid, setter, targetObj = null) {

    }

    getFolderHierarchy(folder) {
        let items = []

        let folderItr = folder.ListFolders()
        let fileItr = folder.ListFiles()
        let currentFolder = folderItr.Next()
        while (IsValidWrappedObject(currentFolder)) {
            items.push(
                <HierarchyItem title={currentFolder.GetName()} regSetter={this.regSetter.bind(this)}>
                    {
                        this.getFolderHierarchy(currentFolder)
                    }
                </HierarchyItem>
            )
            currentFolder = folderItr.Next()
        }

        let currentFile = fileItr.Next()
        while (IsValidWrappedObject(currentFile)) {
            items.push(<HierarchyItem title={currentFile.GetName()}
                                      regSetter={this.regSetter.bind(this)}></HierarchyItem>
            )
        }

        return items
    }

    render() {
        return (
            <HierarchyItem title={"RootFolder"} hierarchyDepth={0}
                           regSetter={this.regSetter.bind(this)}>
                {
                    this.getFolderHierarchy(huahuoEngine.getRootFolder())
                }
            </HierarchyItem>
        )
    }
}

export {FileExplorerX}