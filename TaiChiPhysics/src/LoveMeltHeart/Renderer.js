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

    render(){
        if(this.internalRenderKernel == null){
            this.internalRenderKernel = ti.kernel(()=>{
                for(let I of ndrange(img_size, img_size)){
                    image[I] = [0.067, 0.184, 0.255, 1.0];
                }

                for(let i of range(n_particles)) {
                    if (active[i] == 0)
                        continue

                    let pos = x[i];
                    let ipos = i32(pos * img_size)

                    let this_color = f32([0, 0, 0, 0])
                    if (material[i] == 0) {
                        this_color = [0, 0.5, 0.5, 1.0];
                    } else if (material[i] == 1) {
                        this_color = [0.93, 0.33, 0.23, 1.0];
                    } else if (material[i] == 2) {
                        this_color = [1, 1, 1, 1.0];
                    }

                    // for (let i of ti.static(ti.range(7))) {
                    //     for (let j of ti.static(ti.range(7))) {
                    //         let xoffset = i - 2
                    //         let yoffset = j - 2
                    //         image[ipos + [xoffset, yoffset]] = this_color;
                    //     }
                    // }

                    image[ipos] = this_color;
                }
            })
        }

        this.internalRenderKernel()
        this.canvas.setImage(this.image)
    }
}

export {Renderer}