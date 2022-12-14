
let render_func

class Renderer{
    canvas
    image

    constructor(htmlCanvas, image_size) {
        htmlCanvas.width = image_size
        htmlCanvas.height = image_size

        this.image = ti.Vector.field(4, ti.f32, [image_size, image_size]);
        ti.addToKernelScope({
            image: this.image,
            img_size: image_size
        })

        this.canvas = new ti.Canvas(htmlCanvas)
    }

    // Use x and indices to render
    render(){
        if(render_func == null){
            render_func = ti.kernel(()=>{
                for(let I of ndrange(img_size, img_size)){ // Background color
                    image[I] = [0.067, 0.184, 0.255, 1.0];
                }

                for(let particleIdx of range(num_particles[0])) {
                    let particle_pos = i32(x[particleIdx])
                    for (let i of ti.static(ti.range(7))) {
                        for (let j of ti.static(ti.range(7))) {
                            let xoffset = i - 3
                            let yoffset = j - 3
                            image[particle_pos + [xoffset, yoffset]] = [1.0, 0.0, 0.0, 1.0];
                        }
                    }
                }

            })
        }

        render_func()
        this.canvas.setImage(this.image)
    }
}

export {Renderer}