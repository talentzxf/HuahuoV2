let render_func

class Renderer {
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

    render() {
        if (this.internalRenderKernel == null) {
            this.internalRenderKernel = ti.kernel(() => {
                for (let I of ndrange(img_size, img_size)) {
                    image[I] = [0.067, 0.184, 0.255, 1.0];
                }

                // Draw bricks
                for (let grid_x of range(n_grid)) {
                    for (let grid_y of range(n_grid)) {
                        if (grid_material[grid_x, grid_y] == 1){
                            let img_coordinate = i32([grid_x * dx * img_size, grid_y * dx * img_size])
                            image[img_coordinate] = [1.0, 0.0, 0.0, 1.0]
                        }
                    }
                }

                // Draw particles
                for (let i of range(n_particles)) {
                    if (active[i] == 0)
                        continue

                    let pos = x[i];
                    let ipos = i32(pos * img_size)

                    // for (let i of ti.static(ti.range(7))) {
                    //     for (let j of ti.static(ti.range(7))) {
                    //         let xoffset = i - 2
                    //         let yoffset = j - 2
                    //         image[ipos + [xoffset, yoffset]] = this_color;
                    //     }
                    // }

                    if(ipos[0] >= 0 && ipos[1] >= 0)
                        image[ipos] = particle_color[i];
                }
            })
        }

        this.internalRenderKernel()
        this.canvas.setImage(this.image)
    }
}

export {Renderer}