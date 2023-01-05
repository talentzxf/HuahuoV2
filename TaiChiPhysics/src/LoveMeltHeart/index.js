import {World} from "./World";
import {Renderer} from "./Renderer";
import {Hose} from "./Hose";
import {HeartInCubeShape} from "./HeartInCubeShape";

import {stage} from "./Stages/Stage1";

console.log("Hello Hello")

let image_size = 640

let isPlace = true
function setupEvents(){
    let digButton = document.getElementById("digBrick")
    let placeBrickButton = document.getElementById("placeBrick")

    digButton.addEventListener("click", (evt)=>{
        isPlace = false
    })

    placeBrickButton.addEventListener("click", (evt)=>{
        isPlace = true
    })
}

let win = false

let main = async()=>{
    await ti.init()

    let world = new World()
    let hose = new Hose()
    hose.center = stage.hosePosition
    hose.addToShapeManager(world)

    let heartInCubde = new HeartInCubeShape()
    heartInCubde.center = stage.heartPosition
    heartInCubde.size = stage.heartSize
    heartInCubde.addToShapeManager(world)

    await world.resetSimulation()
    hose.setActivePercentage(0.0)

    // Init bricks
    for(let brick of stage.unremovable_bricks){
        world.addBrick(brick[0], brick[1], 2)
    }

    for(let brick of stage.removable_bricks){
        world.addBrick(brick[0], brick[1], 1)
    }

    for(let pipe of stage.pipes){
        world.addPipe(pipe.start, pipe.end, pipe.displayRectangle.leftup, pipe.displayRectangle.rightdown)
    }

    let htmlCanvas = document.getElementById('result_canvas');
    let renderer = new Renderer(htmlCanvas, image_size)
    await renderer.loadResources()

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

    setupEvents()

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
            heartInCubde.playHeartAnimation(htmlCanvas)
        }

        renderer.render()

        lastDrawTime = currentDrawTime
        requestAnimationFrame(frame)
    }

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