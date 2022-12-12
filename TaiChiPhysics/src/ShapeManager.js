import {quality} from "./Constants";

let n_grid = 128 * quality;
let dx = 1 / n_grid;
let inv_dx = n_grid;
let p_vol = (dx * 0.5) ** 2;
let p_rho = 1;
let p_mass = p_vol * p_rho;
let img_size = 640
let E = 5e3; // Young's modulus a
let nu = 0.2; // Poisson's ratio
let mu_0 = E / (2 * (1 + nu));
let lambda_0 = (E * nu) / ((1 + nu) * (1 - 2 * nu)); // Lame parameters
let dt = 1e-4 / quality;

class MATERIAL_TYPE{
    static LIQUID = 0
    static SOFTBODY = 1
    static SNOW = 2
}

class ShapeManager{
    totalParticles = 0;
    image = ti.Vector.field(4, ti.f32, [img_size, img_size]);
    material
    shapes = []
    canvas
    internalRenderKernel
    sub_step_grid
    sub_step_point

    x // particle positions
    V
    C
    F
    Jp
    grid_v
    grid_m

    constructor() {
        let htmlCanvas = document.getElementById('result_canvas');
        htmlCanvas.width = img_size;
        htmlCanvas.height = img_size;
        this.canvas = new ti.Canvas(htmlCanvas);
    }

    addShape(shape){
        let startParticleIndex = this.totalParticles
        this.totalParticles += shape.totalParticles;

        this.shapes.push(shape)

        return [startParticleIndex, this.totalParticles]
    }

    addParametersToKernel(){
        let _this = this
        let _dt = dt
        ti.addToKernelScope({
            n_particles: _this.totalParticles,
            image: _this.image,
            img_size : img_size,
            x: _this.x,
            v: _this.v,
            C: _this.C,
            F: _this.F,
            material: _this.material,
            Jp: _this.Jp,
            grid_v: _this.grid_v,
            grid_m: _this.grid_m,
            "dt": _dt,
            n_grid,
            dx,
            inv_dx,
            p_vol,
            p_rho,
            p_mass,
            E,
            nu,
            mu_0,
            lambda_0
        })
    }

    resetSimulation(){
        this.material = ti.field(ti.i32, [this.totalParticles]); // material id
        this.x = ti.Vector.field(2, ti.f32, [this.totalParticles]); // position
        this.v = ti.Vector.field(2, ti.f32, [this.totalParticles]); // velocity
        this.C = ti.Matrix.field(2, 2, ti.f32, [this.totalParticles]); // affine vel field
        this.F = ti.Matrix.field(2, 2, ti.f32, this.totalParticles); // deformation gradient
        this.Jp = ti.field(ti.f32, [this.totalParticles]); // plastic deformation
        this.grid_v = ti.Vector.field(2, ti.f32, [n_grid, n_grid]);
        this.grid_m = ti.field(ti.f32, [n_grid, n_grid]);

        this.addParametersToKernel()

        for(let shape of this.shapes){
            shape.reset()
        }
    }

    substep(){
        if(this.sub_step_grid == null){
            this.sub_step_grid = ti.kernel(() => {
                for (let I of ti.ndrange(n_grid, n_grid)) {
                    grid_v[I] = [0, 0];
                    grid_m[I] = 0;
                }
                for (let p of ti.range(n_particles)) {
                    let base = i32(x[p] * inv_dx - 0.5);
                    let fx = x[p] * inv_dx - f32(base);
                    let w = [
                        0.5 * (1.5 - fx) ** 2,
                        0.75 - (fx - 1) ** 2,
                        0.5 * (fx - 0.5) ** 2,
                    ];
                    F[p] = (
                        [
                            [1.0, 0.0],
                            [0.0, 1.0],
                        ] +
                        dt * C[p]
                    ).matmul(F[p]);
                    let h = f32(max(0.1, min(5, ti.exp(10 * (1.0 - Jp[p])))));
                    if (material[p] == 1) {
                        h = 0.3;
                    }
                    let mu = mu_0 * h;
                    let la = lambda_0 * h;
                    if (material[p] == 0) {
                        mu = 0.0;
                    }
                    let svd = ti.svd2D(F[p]);
                    let U = svd.U;
                    let sig = svd.E;
                    let V = svd.V;
                    let J = 1.0;
                    for (let d of ti.static(ti.range(2))) {
                        let new_sig = sig[[d, d]];
                        if (material[p] == 2) {
                            // Plasticity
                            new_sig = min(max(sig[[d, d]], 1 - 2.5e-2), 1 + 4.5e-3);
                        }
                        Jp[p] = (Jp[p] * sig[[d, d]]) / new_sig;
                        sig[[d, d]] = new_sig;
                        J = J * new_sig;
                    }
                    if (material[p] == 0) {
                        F[p] =
                            [
                                [1.0, 0.0],
                                [0.0, 1.0],
                            ] * sqrt(J);
                    } else if (material[p] == 2) {
                        F[p] = U.matmul(sig).matmul(V.transpose());
                    }
                    let stress =
                        (2 * mu * (F[p] - U.matmul(V.transpose()))).matmul(F[p].transpose()) +
                        [
                            [1.0, 0.0],
                            [0.0, 1.0],
                        ] *
                        la *
                        J *
                        (J - 1);
                    stress = -dt * p_vol * 4 * inv_dx * inv_dx * stress;
                    let affine = stress + p_mass * C[p];
                    for (let i of ti.static(ti.range(3))) {
                        for (let j of ti.static(ti.range(3))) {
                            let offset = [i, j];
                            let dpos = (f32(offset) - fx) * dx;
                            let weight = w[[i, 0]] * w[[j, 1]];
                            grid_v[base + offset] +=
                                weight * (p_mass * v[p] + affine.matmul(dpos));
                            grid_m[base + offset] += weight * p_mass;
                        }
                    }
                }

                for (let I of ndrange(n_grid, n_grid)) {
                    let i = I[0];
                    let j = I[1];
                    if (grid_m[I] > 0) {
                        grid_v[I] = (1 / grid_m[I]) * grid_v[I];
                        grid_v[I][1] -= dt * 50;  // Gravity

                        if (i < 3 && grid_v[I][0] < 0) {
                            grid_v[I][0] = 0;
                        }
                        if (i > n_grid - 3 && grid_v[I][0] > 0) {
                            grid_v[I][0] = 0;
                        }
                        if (j < 3 && grid_v[I][1] < 0) {
                            grid_v[I][1] = 0;
                        }
                        if (j > n_grid - 3 && grid_v[I][1] > 0) {
                            grid_v[I][1] = 0;
                        }
                    }
                }
            });

            this.sub_step_point = ti.kernel(()=>{
                for (let p of range(n_particles)) {
                    let base = i32(x[p] * inv_dx - 0.5);
                    let fx = x[p] * inv_dx - f32(base);
                    let w = [
                        0.5 * (1.5 - fx) ** 2,
                        0.75 - (fx - 1.0) ** 2,
                        0.5 * (fx - 0.5) ** 2,
                    ];
                    let new_v = [0.0, 0.0];
                    let new_C = [
                        [0.0, 0.0],
                        [0.0, 0.0],
                    ];
                    for (let i of ti.static(ti.range(3))) {
                        for (let j of ti.static(ti.range(3))) {
                            let dpos = f32([i, j]) - fx;
                            let g_v = grid_v[base + [i, j]];
                            let weight = w[[i, 0]] * w[[j, 1]];
                            new_v = new_v + weight * g_v;
                            new_C = new_C + 4 * inv_dx * weight * g_v.outer_product(dpos);
                        }
                    }
                    v[p] = new_v;
                    C[p] = new_C;
                    x[p] = x[p] + dt * new_v;
                }
            })
        }

        this.sub_step_grid()

        for(let shape of this.shapes){
            shape.apply_constraint()
        }
        this.sub_step_point()


    }

    render(){
        if(this.internalRenderKernel == null){
            this.internalRenderKernel = ti.kernel(()=>{
                for(let I of ndrange(img_size, img_size)){
                    image[I] = [0.067, 0.184, 0.255, 1.0];
                }

                for(let i of range(n_particles)){
                    let pos = x[i];
                    let ipos = i32(pos * img_size)

                    let this_color = f32([0,0,0,0])
                    if (material[i] == 0) {
                        this_color = [0, 0.5, 0.5, 1.0];
                    } else if (material[i] == 1) {
                        this_color = [0.93, 0.33, 0.23, 1.0];
                    } else if (material[i] == 2) {
                        this_color = [1, 1, 1, 1.0];
                    }

                    image[ipos] = this_color;
                }
            })
        }

        this.internalRenderKernel()
        this.canvas.setImage(this.image)
    }
}

export {ShapeManager, quality, MATERIAL_TYPE}