import {ShapeManager} from "./ShapeManager";
import {CircleShape} from "./Shapes";
import {BoardShape} from "./Shapes";
import {dt} from "./Constants";

let main = async() => {
    await ti.init()

    let shapeManager = new ShapeManager()
    let circle = new CircleShape()

    circle.addToShapeManager(shapeManager)

    let board = new BoardShape()
    board.center = [0.5, 0.1]
    board.addToShapeManager(shapeManager)

    let board2 = new BoardShape()
    board2.center = [0.8, 0.1]
    board2.addToShapeManager(shapeManager)

    shapeManager.resetSimulation()

    // let circleShape = new CircleShape()
    //
    // shapeManager.addShape(circleShape)
    //
    // shapeManager.render()

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


//
// let main = async () => {
//     await ti.init();
//
//     let material = ti.field(ti.i32, [n_particles]); // material id
//     let Jp = ti.field(ti.f32, [n_particles]); // plastic deformation
//     let grid_v = ti.Vector.field(2, ti.f32, [n_grid, n_grid]);
//     let grid_m = ti.field(ti.f32, [n_grid, n_grid]);
//
//     let img_size = 640;
//     let image = ti.Vector.field(4, ti.f32, [img_size, img_size]);
//     let group_size = n_particles/ 4;
//
//     let attractor_strength = 1
//     let attractor_pos_kernel = ti.field(ti.f32, [2])
//
//     ti.addToKernelScope({
//         n_particles,
//         n_grid,
//         dx,
//         inv_dx,
//         dt,
//         p_vol,
//         p_rho,
//         p_mass,
//         E,
//         nu,
//         mu_0,
//         lambda_0,
//         x,
//         v,
//         C,
//         F,
//         material,
//         Jp,
//         grid_v,
//         grid_m,
//         image,
//         img_size,
//         group_size,
//         attractor_strength,
//         attractor_pos,
//         center_pos_1,
//         center_pos_2,
//         fixed_point_1,
//         fixed_point_2,
//         attractor_pos_kernel,
//         updateAttractor
//     });
//
//     let substep = ti.kernel(() => {
//         for (let I of ti.ndrange(n_grid, n_grid)) {
//             grid_v[I] = [0, 0];
//             grid_m[I] = 0;
//         }
//         for (let p of ti.range(n_particles)) {
//             let base = i32(x[p] * inv_dx - 0.5);
//             let fx = x[p] * inv_dx - f32(base);
//             let w = [
//                 0.5 * (1.5 - fx) ** 2,
//                 0.75 - (fx - 1) ** 2,
//                 0.5 * (fx - 0.5) ** 2,
//             ];
//             F[p] = (
//                 [
//                     [1.0, 0.0],
//                     [0.0, 1.0],
//                 ] +
//                 dt * C[p]
//             ).matmul(F[p]);
//             let h = f32(max(0.1, min(5, ti.exp(10 * (1.0 - Jp[p])))));
//             if (material[p] == 1) {
//                 h = 0.01;
//             }
//             if(material[p] == 0){
//                 h = 1.0;
//             }
//             let mu = mu_0 * h;
//             let la = lambda_0 * h;
//             // if (material[p] == 0) {
//             //     mu = 0.0;
//             // }
//             let svd = ti.svd2D(F[p]);
//             let U = svd.U;
//             let sig = svd.E;
//             let V = svd.V;
//             let J = 1.0;
//             for (let d of ti.static(ti.range(2))) {
//                 let new_sig = sig[[d, d]];
//                 // if (material[p] == 2) {
//                 //     // Plasticity
//                 //     new_sig = min(max(sig[[d, d]], 1 - 2.5e-2), 1 + 4.5e-3);
//                 // }
//                 Jp[p] = (Jp[p] * sig[[d, d]]) / new_sig;
//                 sig[[d, d]] = new_sig;
//                 J = J * new_sig;
//             }
//             // if (material[p] == 0) {
//             //     F[p] =
//             //         [
//             //             [1.0, 0.0],
//             //             [0.0, 1.0],
//             //         ] * sqrt(J);
//             // } else if (material[p] == 2) {
//             //     F[p] = U.matmul(sig).matmul(V.transpose());
//             // }
//             let stress =
//                 (2 * mu * (F[p] - U.matmul(V.transpose()))).matmul(F[p].transpose()) +
//                 [
//                     [1.0, 0.0],
//                     [0.0, 1.0],
//                 ] *
//                 la *
//                 J *
//                 (J - 1);
//             stress = -dt * p_vol * 4 * inv_dx * inv_dx * stress;
//             let affine = stress + p_mass * C[p];
//             for (let i of ti.static(ti.range(3))) {
//                 for (let j of ti.static(ti.range(3))) {
//                     let offset = [i, j];
//                     let dpos = (f32(offset) - fx) * dx;
//                     let weight = w[[i, 0]] * w[[j, 1]];
//                     grid_v[base + offset] +=
//                         weight * (p_mass * v[p] + affine.matmul(dpos));
//                     grid_m[base + offset] += weight * p_mass;
//                 }
//             }
//         }
//
//         for (let I of ndrange(n_grid, n_grid)) {
//             let i = I[0];
//             let j = I[1];
//             if (grid_m[I] > 0) {
//                 grid_v[I] = (1 / grid_m[I]) * grid_v[I];
//                 grid_v[I][1] -= dt * 50;  // Gravity
//
//                 // let dist = [attractor_pos_x, attractor_pos_y] - dx * [i,j]
//                 //
//                 // grid_v[i, j] += dist / (0.01 + dist.norm()) * dt * 100
//
//                 let dist1 = fixed_point_1 - dx * [i,j]
//                 if(dist1.norm() < 0.01){
//                     grid_v[I][0] = 0;
//                     grid_v[I][1] = 0;
//                 }
//
//                 let dist2 = fixed_point_2 - dx * [i,j]
//                 if(dist2.norm() < 0.01){
//                     grid_v[I][0] = 0;
//                     grid_v[I][1] = 0;
//                 }
//
//                 if (i < 3 && grid_v[I][0] < 0) {
//                     grid_v[I][0] = 0;
//                 }
//                 if (i > n_grid - 3 && grid_v[I][0] > 0) {
//                     grid_v[I][0] = 0;
//                 }
//                 if (j < 3 && grid_v[I][1] < 0) {
//                     grid_v[I][1] = 0;
//                 }
//                 if (j > n_grid - 3 && grid_v[I][1] > 0) {
//                     grid_v[I][1] = 0;
//                 }
//             }
//         }
//         for (let p of range(n_particles)) {
//             let base = i32(x[p] * inv_dx - 0.5);
//             let fx = x[p] * inv_dx - f32(base);
//             let w = [
//                 0.5 * (1.5 - fx) ** 2,
//                 0.75 - (fx - 1.0) ** 2,
//                 0.5 * (fx - 0.5) ** 2,
//             ];
//             let new_v = [0.0, 0.0];
//             let new_C = [
//                 [0.0, 0.0],
//                 [0.0, 0.0],
//             ];
//             for (let i of ti.static(ti.range(3))) {
//                 for (let j of ti.static(ti.range(3))) {
//                     let dpos = f32([i, j]) - fx;
//                     let g_v = grid_v[base + [i, j]];
//                     let weight = w[[i, 0]] * w[[j, 1]];
//                     new_v = new_v + weight * g_v;
//                     new_C = new_C + 4 * inv_dx * weight * g_v.outer_product(dpos);
//                 }
//             }
//             v[p] = new_v;
//             C[p] = new_C;
//             x[p] = x[p] + dt * new_v;
//         }
//     });
//
//     let reset = ti.kernel(() => {
//         for (let i of range(n_particles)) {
//             let group_id = i32(ti.floor(i / group_size));
//
//             let point_x = -1.0
//             let point_y = -1.0
//
//             // Generator points in circle ( 0.5,0.5) radius 0.1
//             let center_pos = center_pos_1
//             let theta = ti.random() * 2.0 * Math.PI
//
//             if(group_id == 2 || group_id == 3){
//                 center_pos = center_pos_2
//             }
//             if(group_id == 0 || group_id == 2){ // Internal
//                 let r = 0.1 * ti.sqrt(ti.random())
//                 point_x = center_pos[0] + r * ti.sin(theta)
//                 point_y = center_pos[1] + r * ti.cos(theta)
//
//                 material[i] = 0;
//             }else{
//                 point_x = center_pos[0] + 0.1 * ti.sin(theta)
//                 point_y = center_pos[1] + 0.1 * ti.cos(theta)
//
//                 material[i] = 1;
//             }
//
//             x[i] = [
//                 point_x,
//                 point_y
//             ];
//
//             v[i] = [0, 0];
//             F[i] = [
//                 [1, 0],
//                 [0, 1],
//             ];
//             Jp[i] = 1.0;
//             C[i] = [
//                 [0, 0],
//                 [0, 0],
//             ];
//         }
//     });
//
//     let render = ti.kernel(() => {
//         updateAttractor()
//
//         let attractor_x = attractor_pos_kernel[0]
//         let attractor_y = attractor_pos_kernel[1]
//
//         for (let I of ndrange(img_size, img_size)) {
//             if(abs(I[0] - attractor_x) + abs(I[1] - attractor_y) < 10.){
//                 image[I] = [1.0, 0.0, 0.0, 1.0];
//             } else{
//                 image[I] = [0.067, 0.184, 0.255, 1.0];
//             }
//
//         }
//         for (let i of range(n_particles)) {
//             let pos = x[i];
//             let ipos = i32(pos * img_size);
//             let this_color = f32([0, 0, 0, 0]);
//             if (material[i] == 0) {
//                 this_color = [0, 0.5, 0.5, 1.0];
//             } else if (material[i] == 1) {
//                 this_color = [0.93, 0.33, 0.23, 1.0];
//             } else if (material[i] == 2) {
//                 this_color = [1, 1, 1, 1.0];
//             }
//             image[ipos] = this_color;
//         }
//     });
//

//
//     htmlCanvas.onclick = function(evt){
//         const rect = htmlCanvas.getBoundingClientRect()
//         const x = evt.clientX - rect.left
//         const y = rect.height - evt.clientY + rect.top
//         attractor_pos = [x, y]
//         console.log("Set attractor_pos at:" + attractor_pos[0] + "," + attractor_pos[1])
//     }
//
//     let canvas = new ti.Canvas(htmlCanvas);
//     reset();
//
//     let i = 0;
//
//     async function frame() {
//         if (window.shouldStop) {
//             return;
//         }
//
//         // for (let i = 0; i < Math.floor(2e-3 / dt); ++i) {
//         //     substep();
//         // }
//
//         render();
//         i = i + 1;
//         canvas.setImage(image);
//         requestAnimationFrame(frame);
//     }
//
//     await frame();
// };
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
