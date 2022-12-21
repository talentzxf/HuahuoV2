import {World} from "./World";
import {Renderer} from "./Renderer";
import {Hose} from "./Hose";

console.log("Hello Hello")

let image_size = 640

let main = async()=>{
    await ti.init()

    let world = new World()

    let hose = new Hose()
    hose.addToShapeManager(world)

    world.resetSimulation()

    let htmlCanvas = document.getElementById('result_canvas');
    let renderer = new Renderer(htmlCanvas, image_size)

    // htmlCanvas.addEventListener("click", (evt)=>{
    //     // Need to have a mapping from screen coordinate to world coordinate.
    //
    //     let mouseX = evt.offsetX
    //     let mouseY = image_size - evt.offsetY
    //
    //     world.addNewParticle(mouseX/ image_size, mouseY/ image_size)
    // })

    let lastDrawTime = Date.now()
    async function frame(){
        let currentDrawTime = Date.now()

        let elapsedMiliseconds = currentDrawTime - lastDrawTime

        let stepCount = 10

        let eachStepTime = elapsedMiliseconds/ stepCount

        for (let i = 0; i < stepCount; ++i) {
            world.substep(eachStepTime/1000.0);
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