let render_func

class Renderer {
    canvas
    image

    pointLineSide() {
        return (l1, l2, x) => {
            return ((l2[0] - l1[0]) * (x[1] - l1[1]) - (l2[1] - l1[0]) * (x[0] - l1[0])) > 0;
        }
    }

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

    fillRectangle() {
        return (p1, p2) => {
            let x0 = p1[0]
            let y0 = p1[1]
            let x1 = p2[0]
            let y1 = p2[1]

            let xmin = min(x0, x1)
            let ymin = min(y0, y1)
            let xmax = max(x0, x1)
            let ymax = max(y0, y1)

            let xWidth = xmax - xmin
            let yHeight = ymax - ymin
            for (let xIndex of range(xWidth)) {
                for (let yIndex of range(yHeight)) {
                    let xPos = xIndex + xmin
                    let yPos = yIndex + ymin

                    let imageIndex = i32([xPos, yPos])
                    image[imageIndex] = [0.0, 1.0, 0.0, 1.0]
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
            plot_line: this.plotLine(),
            fill_rectangle: this.fillRectangle()
        })

        this.canvas = new ti.Canvas(htmlCanvas)
    }

    render() {
        if (this.internalRenderKernel == null) {
            this.internalRenderKernel = ti.kernel(() => {
                for (let I of ndrange(img_size, img_size)) {
                    // image[I] = [0.067, 0.184, 0.255, 1.0];
                    image[I] = [25/255, 39/255, 77/255, 1.0];
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

                    if(material[i] == 0){
                        for (let x_i of ti.static(ti.range(3))) {
                            for (let y_j of ti.static(ti.range(3))) {
                                if( !((x_i == 0 && y_j == 0)
                                    || (x_i == 0 && y_j == 2)
                                    ||(x_i == 2 && y_j == 0)
                                    || (x_i == 2 && y_j == 2)
                                ))
                                {
                                    let xoffset = x_i - 2
                                    let yoffset = y_j - 2

                                    let IPos = ipos + [xoffset, yoffset]
                                    if(IPos[0] > 0 && IPos[1] > 0)
                                        image[IPos] = particle_color[i];
                                }
                            }
                        }
                    }else{
                        if (ipos[0] >= 0 && ipos[1] >= 0)
                            image[ipos] = particle_color[i];
                    }
                }

                // Draw pipes
                for (let i of range(total_pipes[0])) {
                    let start = [pipes[i][0], pipes[i][1]]
                    let end = [pipes[i][2], pipes[i][3]]

                    // Draw based on display info
                    let leftUp = [pipe_display_info[i][0] * img_size, pipe_display_info[i][1] * img_size]
                    let rightDown = [pipe_display_info[i][2] * img_size, pipe_display_info[i][3] * img_size]
                    fill_rectangle(leftUp, rightDown)

                    let startIPos = i32(start * img_size)
                    let endIPos = i32(end * img_size)

                    // Use Bresenham to draw the line
                    plot_line(startIPos, endIPos)
                }
            })
        }

        this.internalRenderKernel()
        this.canvas.setImage(this.image)
    }
}

export {Renderer}