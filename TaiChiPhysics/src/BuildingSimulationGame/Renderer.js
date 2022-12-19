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
            img_size: image_size,
            plotLine: this.plotLine()
        })

        this.canvas = new ti.Canvas(htmlCanvas)
    }

    plotLine() {
        return (p1, p2) => {
            let x0 = p1[0]
            let y0 = p1[1]
            let x1 = p2[0]
            let y1 = p2[1]

            let dx = ti.abs(x1 - x0)
            let sx = 1
            if(x0 >= x1)
                sx = -1

            let dy = -ti.abs(y1 - y0)
            let sy = 1
            if(y0 >= y1)
                sy = -1
            let error = dx + dy
            while(true){
                let imageIndex = i32([x0, y0])
                image[imageIndex] = [1.0, 1.0, 0.0, 1.0]
                if(x0 == x1 && y0 == y1)
                    break

                let e2 = 2 * error
                if(e2 >= dy){
                    if(x0 == x1)
                        break

                    error = error + dy
                    x0 = x0 + sx
                }

                if(e2 <= dx){
                    if(y0 == y1)
                        break;
                    error = error + dx
                    y0 = y0 + sy
                }
            }
        }
    }

    // Use x and indices to render
    render() {
        if (render_func == null) {
            render_func = ti.kernel(() => {
                for (let I of ndrange(img_size, img_size)) { // Background color
                    image[I] = [0.067, 0.184, 0.255, 1.0];
                }

                // Draw particles
                for (let particleIdx of range(num_particles[0])) {
                    let particle_pos = i32(x[particleIdx])
                    for (let i of ti.static(ti.range(7))) {
                        for (let j of ti.static(ti.range(7))) {
                            let xoffset = i - 3
                            let yoffset = j - 3
                            image[particle_pos + [xoffset, yoffset]] = [1.0, 0.0, 0.0, 1.0];
                        }
                    }
                }

                // Draw lines
                for (let particleX of range(num_particles[0])) {
                    let remainingParticles = num_particles[0] - particleX - 1
                    for(let particleYId of range(remainingParticles)){
                        let particleY = particleYId + particleX + 1

                        if(reset_length[particleX, particleY] > 0){
                            let p1position = x[particleX]
                            let p2position = x[particleY]
                            plotLine(p1position, p2position)
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