import i18next from 'i18next'
import HttpApi from 'i18next-http-backend';
import Cache from 'i18next-localstorage-cache';
import LanguageDetector from 'i18next-browser-languagedetector';

function textNodesUnder(el){
    var n, a=[], walk=document.createTreeWalker(el,NodeFilter.SHOW_TEXT,null);
    while(n=walk.nextNode()) a.push(n);
    return a;
}

function afterI18nReady() {
    console.log("HelloWorld:" + i18next.t('helloWorld'))

    let outmostContainer:HTMLElement = document.querySelector("#outmost_container")
    if (outmostContainer.getAttribute("translateAll")){
        let allTextNodes = textNodesUnder(outmostContainer)
        for(let textNode of allTextNodes){
            let textContent = textNode.data.replace(/^\s+|\s+$/g, '')
            if(textContent.length){
                textNode.data = i18next.t(textContent)
                console.log("Replaced:" + textContent + " with:" + textNode.data)
            }
        }
    }
}

i18next
    .use(HttpApi)
    .use(LanguageDetector)
    .init({
        fallbackLng: 'en',
        load: "languageOnly",
        backend: {
            loadPath: '/i18n/{{lng}}-{{ns}}.json'
        }
    }).then(afterI18nReady)