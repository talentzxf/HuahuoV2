import {CustomElement} from "hhcommoncomponents";
import {HHForm} from "../Utilities/HHForm";
import {CSSUtils} from "../Utilities/CSSUtils";
import {huahuoEngine} from "hhenginejs";
import {EventBus, EventNames} from "../Events/GlobalEvents";

@CustomElement({
    selector:"hh-component-list"
})
class ComponentListForm extends HTMLElement implements HHForm{
    selector: string;
    componentListDiv: HTMLElement;

    closeBtn: HTMLElement
    componentListUlContainer: HTMLDivElement
    componentListUL: HTMLUListElement

    connectedCallback(){
        this.style.position = "absolute"
        this.style.top = "50%"
        this.style.left = "50%"
        this.style.transform = "translate(-50%, -50%)"

        this.innerHTML += "<style>" +
            "ul{" +
            "list-style-type: none;" +
            "width: 500px" +
            "}" +
            "li img {" +
            "  float: left;" +
            "  margin: 0 15px 0 0;" +
            "}" +
            "</style>"

        this.componentListDiv = document.createElement("div")
        this.componentListDiv.innerHTML = CSSUtils.formStyle
        this.componentListDiv.innerHTML +=
            "   <form id='componentListForm'>" +
            "   <div style='display: flex; flex-direction: row-reverse'>" +
            "       <div id='componentListCloseBtn' >" +
            "           <img class='far fa-circle-xmark'>" +
            "       </div>" +
            "   </div>" +
            "       <h3>Usable Components</h3>" +
            "       <div id='componentListUlContainer' style='height: 500px; overflow-x: hidden; overflow-y: auto; width: 100%'>" +
            "           <ul id='componentListUl' style='width: 100%'></ul>" +
            "       </div>" +
            "   </form>"

        this.appendChild(this.componentListDiv)

        this.closeBtn = this.componentListDiv.querySelector("#componentListCloseBtn")
        this.closeBtn.addEventListener("mousedown", this.closeForm.bind(this))

        this.componentListUL = this.componentListDiv.querySelector("#componentListUl")

        this.componentListUlContainer = this.querySelector("#componentListUlContainer")
    }

    closeForm() {
        this.style.display = "none"
    }

    updateComponentList(componentNames, targetObj){
        this.componentListUL.innerHTML = i18n.t("component.nothing")
        let ulInnterHTML = "<ul>"
        let componentDivPrefix = "component_"
        for(let componentName of componentNames){
            ulInnterHTML += "<li>"
            ulInnterHTML += " <span>" + componentName + "</span>"
            ulInnterHTML += " <button id='" + componentDivPrefix + componentName + "'>OK</button>"
            ulInnterHTML += "</li>"
        }
        ulInnterHTML += "</ul>"

        let _this = this

        if(componentNames.length > 0){
            this.componentListUL.innerHTML = ulInnterHTML

            let componentAddBtns = this.componentListUL.querySelectorAll("button")
            for(let componentAddBtn of componentAddBtns){
                componentAddBtn.style.width = "30px"
                componentAddBtn.style.padding = "10px"
                let componentName = componentAddBtn.id.split("_")[1]
                componentAddBtn.onclick = function(e){
                    e.preventDefault()

                    let componentConstructor = huahuoEngine.produceObject(componentName)
                    targetObj.addComponent(componentConstructor)

                    EventBus.getInstance().emit(EventNames.COMPONENTADDED, targetObj)
                    _this.closeForm()
                }
            }
        }
    }
}

export {ComponentListForm}