import {World} from "./World";
import {Renderer} from "./Renderer";
import {Hose} from "./Hose";
import {HeartInCubeShape} from "./HeartInCubeShape";
import LoadingImg from "../static/img/loading.gif"

import level1 from "./Levels/Level1.json";
import level2 from "./Levels/Level2.json";

console.log("Hello Hello")

let image_size = 640

let shovelImage = "data:image/svg+xml,%3C%3Fxml version='1.0' encoding='UTF-8'%3F%3E%3Csvg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' width='16px' height='16px' viewBox='0 0 16 16' version='1.1'%3E%3Cg id='surface1'%3E%3Cpath style=' stroke:none%3Bfill-rule:nonzero%3Bfill:rgb(92.941176%25 92.941176%25 92.941176%25)%3Bfill-opacity:1%3B' d='M 2.894531 7.542969 L 1.507812 8.933594 C -0.128906 10.566406 -0.269531 13.128906 1.074219 14.925781 C 2.871094 16.269531 5.433594 16.128906 7.066406 14.492188 L 8.457031 13.105469 Z M 2.894531 7.542969 '/%3E%3Cpath style=' stroke:none%3Bfill-rule:nonzero%3Bfill:rgb(85.882353%25 85.882353%25 85.882353%25)%3Bfill-opacity:1%3B' d='M 1.074219 14.925781 C 2.871094 16.269531 5.433594 16.128906 7.066406 14.492188 L 8.457031 13.105469 L 5.675781 10.324219 Z M 1.074219 14.925781 '/%3E%3Cpath style=' stroke:none%3Bfill-rule:nonzero%3Bfill:rgb(74.901961%25 66.27451%25 57.647059%25)%3Bfill-opacity:1%3B' d='M 12.195312 4.730469 L 3.625 13.300781 C 3.367188 13.558594 2.953125 13.558594 2.699219 13.300781 C 2.441406 13.046875 2.441406 12.632812 2.699219 12.375 L 11.269531 3.804688 Z M 12.195312 4.730469 '/%3E%3Cpath style=' stroke:none%3Bfill-rule:nonzero%3Bfill:rgb(69.803922%25 60.392157%25 49.411765%25)%3Bfill-opacity:1%3B' d='M 2.699219 13.300781 C 2.953125 13.558594 3.367188 13.558594 3.625 13.300781 L 11.964844 4.960938 L 11.5 4.5 Z M 2.699219 13.300781 '/%3E%3Cpath style=' stroke:none%3Bfill-rule:nonzero%3Bfill:rgb(92.941176%25 92.941176%25 92.941176%25)%3Bfill-opacity:1%3B' d='M 13.785156 4.070312 C 13.273438 4.582031 12.445312 4.582031 11.933594 4.070312 C 11.417969 3.554688 11.417969 2.726562 11.933594 2.214844 L 11.238281 1.519531 C 10.339844 2.414062 10.339844 3.867188 11.238281 4.761719 C 12.132812 5.660156 13.585938 5.660156 14.480469 4.761719 Z M 13.785156 4.070312 '/%3E%3Cpath style=' stroke:none%3Bfill-rule:nonzero%3Bfill:rgb(85.882353%25 85.882353%25 85.882353%25)%3Bfill-opacity:1%3B' d='M 11.933594 4.070312 L 11.238281 4.761719 C 12.132812 5.660156 13.585938 5.660156 14.480469 4.761719 L 13.785156 4.066406 C 13.273438 4.582031 12.445312 4.582031 11.933594 4.070312 Z M 11.933594 4.070312 '/%3E%3Cpath style=' stroke:none%3Bfill-rule:nonzero%3Bfill:rgb(74.901961%25 66.27451%25 57.647059%25)%3Bfill-opacity:1%3B' d='M 15.753906 3.953125 L 14.597656 5.109375 L 10.890625 1.402344 L 12.046875 0.246094 Z M 15.753906 3.953125 '/%3E%3Cpath style=' stroke:none%3Bfill-rule:nonzero%3Bfill:rgb(69.803922%25 60.392157%25 49.411765%25)%3Bfill-opacity:1%3B' d='M 15.753906 3.953125 L 14.597656 5.109375 L 12.742188 3.257812 L 13.902344 2.097656 Z M 15.753906 3.953125 '/%3E%3Cpath style=' stroke:none%3Bfill-rule:nonzero%3Bfill:rgb(18.039216%25 17.647059%25 19.215686%25)%3Bfill-opacity:1%3B' d='M 15.929688 3.777344 L 12.222656 0.0703125 C 12.125 -0.0234375 11.96875 -0.0234375 11.875 0.0703125 L 10.714844 1.230469 C 10.667969 1.277344 10.644531 1.339844 10.644531 1.402344 C 10.644531 1.46875 10.667969 1.53125 10.714844 1.578125 L 10.78125 1.644531 C 10.160156 2.402344 10.039062 3.4375 10.425781 4.296875 L 5.296875 9.429688 L 3.152344 7.285156 C 3.058594 7.191406 2.902344 7.191406 2.804688 7.285156 L 1.414062 8.675781 C 0.585938 9.503906 0.0898438 10.601562 0.0117188 11.769531 C -0.0664062 12.921875 0.269531 14.066406 0.960938 14.988281 C 0.976562 15.007812 0.992188 15.023438 1.011719 15.039062 C 1.847656 15.664062 2.863281 16 3.90625 16 C 4.015625 16 4.121094 15.996094 4.230469 15.988281 C 5.398438 15.910156 6.496094 15.414062 7.324219 14.585938 L 8.714844 13.195312 C 8.808594 13.097656 8.808594 12.941406 8.714844 12.847656 L 6.570312 10.703125 L 11.699219 5.574219 C 12.023438 5.71875 12.378906 5.796875 12.742188 5.796875 C 13.339844 5.796875 13.902344 5.59375 14.355469 5.21875 L 14.421875 5.285156 C 14.46875 5.332031 14.53125 5.355469 14.597656 5.355469 C 14.660156 5.355469 14.722656 5.332031 14.769531 5.285156 L 15.929688 4.125 C 15.972656 4.082031 16 4.019531 16 3.953125 C 16 3.886719 15.972656 3.824219 15.929688 3.777344 Z M 11.835938 2.699219 L 13.300781 4.164062 C 13.136719 4.265625 12.941406 4.324219 12.742188 4.324219 C 12.457031 4.324219 12.191406 4.210938 11.988281 4.011719 C 11.789062 3.808594 11.675781 3.542969 11.675781 3.257812 C 11.675781 3.058594 11.734375 2.863281 11.835938 2.699219 Z M 8.191406 13.019531 L 6.976562 14.238281 C 5.464844 15.746094 3.046875 15.929688 1.335938 14.664062 C 0.0703125 12.953125 0.253906 10.535156 1.761719 9.023438 L 2.980469 7.808594 L 4.949219 9.777344 L 2.605469 12.117188 C 2.4375 12.289062 2.34375 12.515625 2.34375 12.757812 C 2.34375 12.996094 2.4375 13.222656 2.605469 13.394531 C 2.78125 13.570312 3.011719 13.65625 3.242188 13.65625 C 3.472656 13.65625 3.707031 13.570312 3.882812 13.394531 L 6.222656 11.050781 Z M 3.535156 13.046875 C 3.375 13.207031 3.113281 13.207031 2.953125 13.046875 C 2.792969 12.886719 2.792969 12.625 2.953125 12.464844 L 10.679688 4.738281 C 10.761719 4.847656 10.847656 4.953125 10.945312 5.054688 C 11.046875 5.152344 11.148438 5.238281 11.261719 5.320312 Z M 12.742188 5.304688 C 12.195312 5.304688 11.679688 5.09375 11.292969 4.707031 C 10.554688 3.964844 10.5 2.796875 11.132812 1.992188 L 11.480469 2.34375 C 11.289062 2.609375 11.1875 2.925781 11.1875 3.257812 C 11.1875 3.671875 11.347656 4.0625 11.640625 4.359375 C 11.9375 4.652344 12.328125 4.8125 12.742188 4.8125 C 13.074219 4.8125 13.390625 4.710938 13.65625 4.519531 L 14.007812 4.867188 C 13.648438 5.152344 13.207031 5.304688 12.742188 5.304688 Z M 14.597656 4.761719 L 11.238281 1.402344 L 12.046875 0.59375 L 15.40625 3.953125 Z M 14.597656 4.761719 '/%3E%3C/g%3E%3C/svg%3E"

let isPlace = true
function setupEvents(htmlCanvas, brickImage){
    let digButton = document.getElementById("digBrick")
    let placeBrickButton = document.getElementById("placeBrick")

    digButton.addEventListener("click", (evt)=>{
        isPlace = false

        htmlCanvas.style.cursor = "url(\"" + shovelImage + "\") 0 16, pointer"
    })

    placeBrickButton.addEventListener("click", (evt)=>{
        isPlace = true
        htmlCanvas.style.cursor = "url(\"" + brickImage + "\") 0 16, pointer"
    })
}

let win = false
let currentLevel = ""

let main = async()=>{
    await ti.init()

    let world = new World()
    let hose = new Hose()

    let queryString = window.location.search
    const urlParams = new URLSearchParams(queryString)
    currentLevel = urlParams.get("level")

    let level = level1
    if(currentLevel == "level2"){
        level = level2
    } else {
        currentLevel = "level1"
    }

    hose.center = level.hosePosition
    hose.velocity = level.hoseVelocity
    hose.addToShapeManager(world)

    let heartInCubde = new HeartInCubeShape()
    heartInCubde.center = level.heartPosition
    heartInCubde.size = level.heartSize
    heartInCubde.addToShapeManager(world)

    await world.resetSimulation()
    hose.setActivePercentage(0.0)

    // Init bricks
    for(let brick of level.unremovable_bricks){
        world.addBrick(brick[0], brick[1], 2)
    }

    for(let brick of level.removable_bricks){
        world.addBrick(brick[0], brick[1], 1)
    }

    for(let pipe of level.pipes){
        world.addPipe(pipe.start, pipe.end, pipe.displayRectangle.leftup, pipe.displayRectangle.rightdown)
    }

    let htmlCanvas = document.getElementById('result_canvas');
    let renderer = new Renderer(htmlCanvas, image_size, level.bgImgUrl)
    await renderer.loadResources()

    htmlCanvas.style.cursor = "url(\"" + renderer.brickImageSmallUrl + "\"), pointer"

    htmlCanvas.addEventListener("click", (evt)=>{
        // Need to have a mapping from screen coordinate to world coordinate.

        let mouseX = evt.offsetX
        let mouseY = image_size - evt.offsetY

        if(!win){
            if(isPlace)
                world.addBrick(mouseX/ image_size, mouseY/ image_size)
            else
                world.removeBrick(mouseX/ image_size, mouseY/ image_size)
        }
    })

    setupEvents(htmlCanvas, renderer.brickImageSmallUrl)

    let lastDrawTime = Date.now()

    function gameWin(){
        win = true
        world.removeAllParticlesExceptHeart()
    }

    async function frame(){
        let currentDrawTime = Date.now()

        let elapsedMiliseconds = currentDrawTime - lastDrawTime

        let stepCount = 10

        let eachStepTime = elapsedMiliseconds/ stepCount

        if(!win){
            for (let i = 0; i < stepCount; ++i) {
                world.substep(eachStepTime/1000.0);
            }

            world.update()

            world.hasWon().then((val)=>{
                if(val <= 0){
                    gameWin()
                }
            })
        }else{
            heartInCubde.playHeartAnimation(htmlCanvas, currentLevel)
        }

        renderer.render()

        lastDrawTime = currentDrawTime
        requestAnimationFrame(frame)
    }

    let loadingDiv = document.body.querySelector("#loading")
    let contentDiv = document.body.querySelector("#content")
    loadingDiv.style.display = "none"
    contentDiv.style.display = "flex"
    await frame()

    console.log("HiHi")
}




// This is just because StackBlitz has some weird handling of external scripts.
// Normally, you would just use `<script src="https://unpkg.com/taichi.js/dist/taichi.umd.js"></script>` in the HTML
const script = document.createElement('script');
script.addEventListener('load', function () {
    main();
});
// script.src = 'https://unpkg.com/taichi.js/dist/taichi.umd.js';
script.src = 'https://unpkg.com/taichi.js/dist/taichi.dev.umd.js';

// Append to the `head` element
document.head.appendChild(script);