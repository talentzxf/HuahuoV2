import {CustomElement} from "hhcommoncomponents";
import {huahuoEngine} from "hhenginejs";
import {Logger} from "hhcommoncomponents";
import {SceneView} from "../SceneView/SceneView";
import {HHTimeline} from "hhtimeline"
import {saveAs} from 'file-saver';

declare var Module:any

function save() {
    // Restore current scene view.

    let mainSceneView:SceneView = document.querySelector("#mainScene")
    let oldStoreId = huahuoEngine.GetCurrentStoreId()

    try{
        console.log("Setting default store by index:" + mainSceneView.storeId)
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(mainSceneView.storeId)
        let Uint8Array = Module.writeObjectStoreInMemoryFile()
        let blob = new Blob([Uint8Array], {type: "application/octet-stream"})
        saveAs(blob, "huahuo.data")
        Logger.info("Good!!")
    }finally {
        console.log("Setting default store by index asdfasdfasdf:" + oldStoreId)
        huahuoEngine.GetDefaultObjectStoreManager().SetDefaultStoreByIndex(oldStoreId)
    }
}

function load(fName:string, e) {
    Logger.info("Opening:" + fName)

    let fileName = fName.split("\\").pop();
    let file = e.target.files[0];
    let reader = new FileReader()
    reader.onload = function (e:ProgressEvent<FileReader>) {
        let arrayBuffer = e.target.result
        let fileContent = new Uint8Array(arrayBuffer as ArrayBuffer);
        let storeMemoryFile = "mem://" + fileName;
        let fileSize = fileContent.length;
        let memoryFileContent = Module.createMemFile(storeMemoryFile, fileSize);
        for (let i = 0; i < fileSize; i++) { // Copy to the file, byte by byte
            memoryFileContent[i] = fileContent[i];
        }

        let result = Module.LoadStoreFileCompletely(storeMemoryFile);
        if (result == 0) {
            let timeline:HHTimeline = document.querySelector("hh-timeline")
            timeline.reloadTracks();
        } else {
            console.log("Can't load file: " + storeMemoryFile)
        }
    }
    reader.readAsArrayBuffer(file)
}

@CustomElement({
    selector: "hh-tool-bar"
})
class HHToolBar extends HTMLElement{
    saveButton: HTMLButtonElement
    loadButton: HTMLButtonElement
    constructor() {
        super();

        //                <button onclick="window.menuoperations.save()">save</button>
        //                 <input type="file" onchange="window.menuoperations.load(this.value, event)">
        //                 <button onclick="window.menuoperations.uploadAndOpenPlayer()">uploadAndOpen</button>

        this.saveButton = document.createElement("button")
        this.saveButton.style.width = "30px"
        this.saveButton.style.height = "30px"
        this.saveButton.innerHTML = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n" +
            "<!-- Generator: Adobe Illustrator 19.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\n" +
            "<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n" +
            "\t viewBox=\"0 0 446.074 446.074\" style=\"enable-background:new 0 0 446.074 446.074;\" xml:space=\"preserve\">\n" +
            "<g>\n" +
            "\t<polygon style=\"fill:#003F8A;\" points=\"190.29,97.437 190.29,198.327 255.02,198.327 297.119,156.229 361.164,92.183 \n" +
            "\t\t380.58,111.598 380.58,86.257 302.36,8.037 302.36,97.437 \t\"/>\n" +
            "\t<polygon style=\"fill:#003F8A;\" points=\"302.36,388.617 380.58,388.617 380.58,242.586 302.36,320.806 \t\"/>\n" +
            "\t<polygon style=\"fill:#9BC9FF;\" points=\"190.29,198.327 190.29,263.057 255.02,198.327 \t\"/>\n" +
            "\t<path style=\"fill:#9BC9FF;\" d=\"M204.266,388.617l-13.976-13.976L204.266,388.617z\"/>\n" +
            "\t<polygon style=\"fill:#9BC9FF;\" points=\"234.549,388.617 302.36,388.617 302.36,320.806 \t\"/>\n" +
            "\t<rect x=\"190.29\" y=\"8.037\" style=\"fill:#9BC9FF;\" width=\"112.07\" height=\"89.4\"/>\n" +
            "\t<polygon style=\"fill:#BDDBFF;\" points=\"190.29,263.057 190.29,198.327 78.22,198.327 78.22,388.617 114.433,388.617 \n" +
            "\t\t134.498,318.849 \t\"/>\n" +
            "\t<polygon style=\"fill:#BDDBFF;\" points=\"190.29,374.641 176.953,361.304 134.498,318.849 \t\"/>\n" +
            "\t<polygon style=\"fill:#005ECE;\" points=\"190.29,198.327 190.29,97.437 78.22,97.437 78.22,8.037 0,8.037 0,388.617 78.22,388.617 \n" +
            "\t\t78.22,198.327 \t\"/>\n" +
            "\t<rect x=\"78.22\" y=\"8.037\" style=\"fill:#BDDBFF;\" width=\"112.07\" height=\"89.4\"/>\n" +
            "\t<polygon style=\"fill:#FFDA44;\" points=\"255.02,198.327 190.29,263.057 134.498,318.849 176.953,361.304 339.573,198.683 \n" +
            "\t\t297.119,156.229 \t\"/>\n" +
            "\t<polygon style=\"fill:#FFCD00;\" points=\"339.573,198.683 176.953,361.304 190.29,374.641 204.266,388.617 219.407,403.759 \n" +
            "\t\t382.028,241.138 \t\"/>\n" +
            "\t<polygon style=\"fill:#FFEB99;\" points=\"134.498,318.849 114.433,388.617 100.22,438.037 219.407,403.759 \t\"/>\n" +
            "\t<polygon style=\"fill:#2488FF;\" points=\"297.119,156.229 339.573,198.683 403.619,134.637 446.074,177.092 380.58,111.598 \n" +
            "\t\t361.164,92.183 \t\"/>\n" +
            "\t\n" +
            "\t\t<rect x=\"362.804\" y=\"142.6\" transform=\"matrix(-0.7071 -0.7071 0.7071 -0.7071 537.7359 598.5123)\" style=\"fill:#005ECE;\" width=\"60.04\" height=\"90.575\"/>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "</svg>\n"

        this.saveButton.addEventListener("click", save)
        this.appendChild(this.saveButton)

        this.loadButton = document.createElement("button")
        this.loadButton.style.width = "30px"
        this.loadButton.style.height = "30px"
        this.loadButton.innerHTML = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n" +
            "<!-- Generator: Adobe Illustrator 19.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\n" +
            "<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\n" +
            "\t viewBox=\"0 0 485.943 485.943\" style=\"enable-background:new 0 0 485.943 485.943;\" xml:space=\"preserve\">\n" +
            "<g>\n" +
            "\t<path style=\"fill:#DDB200;\" d=\"M475.356,119.703c-8.516-9.998-21.277-15.731-35.011-15.731H195.348\n" +
            "\t\tc2.719,5.187,4.624,10.834,5.496,16.777l7.801,53.222h140.258c25.946,0,50.406,19.899,55.686,45.303l40.05,192.698l40.683-255.202\n" +
            "\t\tC487.505,143.211,483.872,129.7,475.356,119.703z\"/>\n" +
            "\t<path style=\"fill:#FFCD00;\" d=\"M348.902,173.971H208.645l34.885,238.001h201.109l-40.05-192.698\n" +
            "\t\tC399.309,193.87,374.849,173.971,348.902,173.971z\"/>\n" +
            "\t<path style=\"fill:#FFDA44;\" d=\"M200.844,120.749c-0.872-5.944-2.777-11.59-5.496-16.777c-9.281-17.705-28.145-30-48.649-30H45.949\n" +
            "\t\tc-13.68,0-26.452,5.702-35.042,15.645c-8.589,9.941-12.376,23.407-10.389,36.943L42.42,411.971h201.109l-34.885-238.001\n" +
            "\t\tL200.844,120.749z\"/>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "<g>\n" +
            "</g>\n" +
            "</svg>\n"

        this.loadButton.addEventListener("click", this.onFileSelected.bind(this))
        this.appendChild(this.loadButton)
    }

    onFileSelected(){
        let hiddenFileButton = document.createElement("input")
        hiddenFileButton.type = "file"
        hiddenFileButton.click()

        hiddenFileButton.addEventListener("change", (evt)=>{
            let fName = hiddenFileButton.value
            load(fName, evt)
        })
    }
}

export {HHToolBar}