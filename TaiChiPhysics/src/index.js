import {ShapeManager} from "./ShapeManager";
import {CircleShape} from "./Shapes";
import {BoardShape} from "./Shapes";
import {dt} from "./Constants";

let main = async() => {
    await ti.init()

    let shapeManager = new ShapeManager()

    let circle = new CircleShape()
    circle.addToShapeManager(shapeManager)

    let board2 = new BoardShape()
    board2.center = [0.8, 0.1]
    board2.addToShapeManager(shapeManager)


    // let circleShape = new CircleShape()
    //
    // shapeManager.addShape(circleShape)
    //
    // shapeManager.render()

    shapeManager.resetSimulation()

    let lastBoardCreateTime = Date.now()

    let currentBoards = 1
    let totalBoards = 3

    async function frame() {
        if (window.shouldStop) {
            return;
        }

        for (let i = 0; i < Math.floor(2e-3 / dt); ++i) {
            shapeManager.substep();
        }

        shapeManager.render();
        requestAnimationFrame(frame);
    }

    await frame();

    console.log("Hello")
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
