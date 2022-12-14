let update_indices
let add_particle

class World {
    sprint_Y = 1000 // Spring's modulus
    drag_damping = 1
    dashpot_damping = 100

    max_num_particles = 1024
    particle_mass = 1.0
    dt = 1e-3
    substeps = 10

    num_particles = ti.field(ti.i32, [1])

    x = ti.Vector.field(2, ti.f32, [this.max_num_particles])
    v = ti.Vector.field(2, ti.f32, [this.max_num_particles])
    f = ti.Vector.field(2, ti.f32, [this.max_num_particles])

    indices = ti.field(ti.i32, [this.max_num_particles * this.max_num_particles * 2])

    per_vertex_color = ti.Vector.field(3, ti.f32, [this.max_num_particles])

    reset_length = ti.field(ti.f32, [this.max_num_particles, this.max_num_particles])

    constructor() {
        let resultObj = {}

        let keys = Object.getOwnPropertyNames(this)
        for(let key of keys){
            resultObj[key] = this[key]
        }

        ti.addToKernelScope(resultObj)
    }

    addNewParticle(posX, posY){
        if(add_particle == null){
            add_particle = ti.kernel((posX, posY)=>{
                let new_particle_id = num_particles[0]
                x[new_particle_id] = [posX, posY]
                v[new_particle_id] = [0, 0]
                num_particles[0] = num_particles[0] + 1

                for( let i of range(new_particle_id)){
                    let dist = (x[new_particle_id] - x[i]).norm()
                    let connection_radius = 0.15
                    if(dist < connection_radius){
                        reset_length[i, new_particle_id] = 0.1
                        reset_length[new_particle_id, i] = 0.1
                    }
                }
            })
        }

        add_particle(posX, posY)
    }

    updateIndices(){
        if(update_indices == null){
            update_indices = ti.kernel(()=>{
                for(let i of range(max_num_particles)){
                    indices[i] = max_num_particles - 1
                }

                let totalUnusedParticleCount = max_num_particles - num_particles[0]
                for(let i of range(totalUnusedParticleCount)){
                    let unUsedIdx = i + max_num_particles
                    x[unUsedIdx] = [-1, 1]
                }

                for(let i of range(num_particles[0])){
                    per_vertex_color[i] = [0,0,0]
                }

                for(let i of range(num_particles[0])){
                    let jCount = num_particles[0] - i
                    for(let jIdx of range(jCount)){
                        let j = jIdx - i

                        let line_id = i * max_num_particles + j
                        if(reset_length[i, j] != 0){
                            indices[line_id*2] = i
                            indices[line_id*2 + 1] = i
                        }
                    }
                }
            })
        }

        update_indices()
    }
}

export {World}