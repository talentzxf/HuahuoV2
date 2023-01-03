let render_func

class Renderer {
    canvas
    image

    // Bresenham's line algorithm
    plotLine() {
        return (p1, p2) => {
            let x0 = p1[0]
            let y0 = p1[1]
            let x1 = p2[0]
            let y1 = p2[1]

            let dx = ti.abs(x1 - x0)
            let sx = 1
            if (x0 >= x1)
                sx = -1

            let dy = -ti.abs(y1 - y0)
            let sy = 1
            if (y0 >= y1)
                sy = -1
            let error = dx + dy
            while (true) {
                let imageIndex = i32([x0, y0])
                image[imageIndex] = [1.0, 1.0, 0.0, 1.0]
                if (x0 == x1 && y0 == y1)
                    break

                let e2 = 2 * error
                if (e2 >= dy) {
                    if (x0 == x1)
                        break

                    error = error + dy
                    x0 = x0 + sx
                }

                if (e2 <= dx) {
                    if (y0 == y1)
                        break;
                    error = error + dx
                    y0 = y0 + sy
                }
            }
        }
    }

    constructor(htmlCanvas, image_size) {
        htmlCanvas.width = image_size
        htmlCanvas.height = image_size

        this.image = ti.Vector.field(4, ti.f32, [image_size, image_size]);
        ti.addToKernelScope({
            image: this.image,
            img_size: image_size,
            plot_line: this.plotLine()
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
                        let img_coordinate = i32([grid_x * dx * img_size, grid_y * dx * img_size])
                        if (grid_material[grid_x, grid_y] == 2) {
                            image[img_coordinate] = [1.0, 0.0, 0.0, 1.0]
                        } else if (grid_material[grid_x, grid_y] == 1) {
                            image[img_coordinate] = [0.0, 1.0, 0.0, 1.0]
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

                    if (ipos[0] >= 0 && ipos[1] >= 0)
                        image[ipos] = particle_color[i];
                }

                // Draw pipes
                for (let i of range(total_pipes[0])) {
                    let start = [pipes[i][0], pipes[i][1]]
                    let end = [pipes[i][2], pipes[i][3]]

                    let startIPos = i32(start * img_size)
                    let endIPos = i32(end * img_size)

                    // Use Bresenham to draw the line
                    plot_line(startIPos, endIPos)
                }

                return total_pipes[0]
            })
        }

        this.internalRenderKernel().then((val) => {
            console.log("Rendered total pipes:" + val)
        })
        this.canvas.setImage(this.image)
    }
}

export {Renderer}