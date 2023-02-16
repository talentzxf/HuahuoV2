let update_indices
let add_particle
let sub_step
let has_won_kernel

class World {
    sprint_Y = 0.0001 // Spring's modulus
    drag_damping = 1
    dashpot_damping = 100

    max_num_particles = 1024
    particle_mass = 1.0

    num_particles = ti.field(ti.i32, [1])

    x = ti.Vector.field(2, ti.f32, [this.max_num_particles])
    v = ti.Vector.field(2, ti.f32, [this.max_num_particles])
    f = ti.Vector.field(2, ti.f32, [this.max_num_particles])

    per_vertex_color = ti.Vector.field(3, ti.f32, [this.max_num_particles])

    reset_length = ti.field(ti.f32, [this.max_num_particles, this.max_num_particles])

    constructor() {
        let resultObj = {}

        let keys = Object.getOwnPropertyNames(this)
        for (let key of keys) {
            resultObj[key] = this[key]
        }

        ti.addToKernelScope(resultObj)
    }

    substep(elapsedTime) {
        if (sub_step == null) {
            sub_step = ti.kernel((delta_time) => {
                let n = num_particles[0]

                let spring_force = [-1.0, -1.0]

                for (let i of range(n)) {
                    // Gravity
                    // f[i] = [0, -9.8] * particle_mass

                    for(let j of range(n)){
                        if(reset_length[i,j] > 0 && i != j ){
                            let x_ij = x[i] - x[j]
                            let d = x_ij.normalized()


                            ti.atomicAdd(f[i], -sprint_Y *(x_ij.norm() / reset_length[i,j] - 1) * d)
                            spring_force = d
                            // spring_force = -sprint_Y *(x_ij.norm() / reset_length[i,j] - 1) * d
                            //
                            // let v_rel = (v[i] - v[j]).dot(d)
                            // f[i] += -dashpot_damping * v_rel * d
                        }
                    }
                }

                for (let i of range(n)) {
                    v[i] += delta_time * f[i] / particle_mass

                    v[i] *= ti.exp(-delta_time * drag_damping)
                    x[i] += v[i] * delta_time

                    for(let d of ti.static(range(2))){
                        if(x[i][d] < 0){
                            x[i][d] = 0
                            v[i][d] = 0
                        }

                    if(x[i][d] > 1){
                        x[i][d] = 1
                        v[i][d] = 0
                    }
                    }
                }

                return spring_force
            })
        }

        sub_step(elapsedTime).then((val)=>{
            console.log(val)
        })
    }

    addNewParticle(posX, posY) {
        if (add_particle == null) {
            add_particle = ti.kernel((posX, posY) => {
                let new_particle_id = num_particles[0]
                x[new_particle_id] = [posX, posY]
                v[new_particle_id] = [0.0, 0]
                num_particles[0] = num_particles[0] + 1

                for (let i of range(new_particle_id)) {
                    let dist = (x[new_particle_id] - x[i]).norm()
                    let connection_radius = 0.1
                    if (dist < connection_radius) {
                        reset_length[i, new_particle_id] = 0.1
                        reset_length[new_particle_id, i] = 0.1
                    }
                }

            })
        }

        add_particle(posX, posY)
    }
}

export {World}