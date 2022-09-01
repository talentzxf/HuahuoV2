let i18n = (window as any).i18n

function textNodesUnder(el){
    var n, a=[], walk=document.createTreeWalker(el,NodeFilter.SHOW_TEXT,null);
    while(n=walk.nextNode()) a.push(n);
    return a;
}

function afterI18nReady() {
    console.log("HelloWorld:" + i18n.t('helloWorld'))

    let outmostContainer:HTMLElement = document.querySelector("#outmost_container")
    if (outmostContainer.getAttribute("translateAll")){
        let allTextNodes = textNodesUnder(outmostContainer)
        for(let textNode of allTextNodes){
            let textContent = textNode.data.replace(/^\s+|\s+$/g, '')
            if(textContent.length){
                textNode.data = i18n.t(textContent)
                console.log("Replaced:" + textContent + " with:" + textNode.data)
            }
        }
    }
}

i18n.ExecuteAfterInited(afterI18nReady)
