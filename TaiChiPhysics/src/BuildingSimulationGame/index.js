import {World} from "./World";
import {Renderer} from "./Renderer";

console.log("Hello Hello")

let image_size = 640

let main = async()=>{
    await ti.init()

    let world = new World()

    let htmlCanvas = document.getElementById('result_canvas');
    let renderer = new Renderer(htmlCanvas, image_size)

    world.updateIndices()

    document.body.addEventListener("click", (evt)=>{
        let mouseX = evt.offsetX
        let mouseY = image_size - evt.offsetY

        world.addNewParticle(mouseX, mouseY)
    })

    async function frame(){
        world.updateIndices()
        renderer.render()
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