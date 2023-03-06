let render_func

class Renderer {
    canvas
    image

    raw_bg_image

    hiddenCanvas
    brickImg

    brickImageUrl = "data:image/svg+xml,%3C%3Fxml version='1.0' encoding='iso-8859-1'%3F%3E%3C!-- Uploaded to: SVG Repo  www.svgrepo.com  Generator: SVG Repo Mixer Tools --%3E%3Csvg version='1.1' id='Layer_1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'  viewBox='0 0 512 512' xml:space='preserve'%3E%3Crect style='fill:%23FFCC67%3B' width='512' height='512'/%3E%3Cg%3E%3Crect y='165.336' style='fill:%23E3AA4D%3B' width='512' height='16'/%3E%3Crect y='330.64' style='fill:%23E3AA4D%3B' width='512' height='16'/%3E%3Crect x='248' style='fill:%23E3AA4D%3B' width='16' height='173.336'/%3E%3Crect x='248' y='338.64' style='fill:%23E3AA4D%3B' width='16' height='173.336'/%3E%3Crect x='124.256' y='173.656' style='fill:%23E3AA4D%3B' width='16' height='164.696'/%3E%3Crect x='371.76' y='174.56' style='fill:%23E3AA4D%3B' width='16' height='164.696'/%3E%3C/g%3E%3C/svg%3E"
    brickImageSmallUrl = "data:image/jpeg;base64,/9j/4AAQSkZJRgABAQAAAQABAAD/4gHYSUNDX1BST0ZJTEUAAQEAAAHIAAAAAAQwAABtbnRyUkdCIFhZWiAAAAAAAAAAAAAAAABhY3NwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAA9tYAAQAAAADTLQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAlkZXNjAAAA8AAAACRyWFlaAAABFAAAABRnWFlaAAABKAAAABRiWFlaAAABPAAAABR3dHB0AAABUAAAABRyVFJDAAABZAAAAChnVFJDAAABZAAAAChiVFJDAAABZAAAAChjcHJ0AAABjAAAADxtbHVjAAAAAAAAAAEAAAAMZW5VUwAAAAgAAAAcAHMAUgBHAEJYWVogAAAAAAAAb6IAADj1AAADkFhZWiAAAAAAAABimQAAt4UAABjaWFlaIAAAAAAAACSgAAAPhAAAts9YWVogAAAAAAAA9tYAAQAAAADTLXBhcmEAAAAAAAQAAAACZmYAAPKnAAANWQAAE9AAAApbAAAAAAAAAABtbHVjAAAAAAAAAAEAAAAMZW5VUwAAACAAAAAcAEcAbwBvAGcAbABlACAASQBuAGMALgAgADIAMAAxADb/2wBDAAgGBgcGBQgHBwcJCQgKDBQNDAsLDBkSEw8UHRofHh0aHBwgJC4nICIsIxwcKDcpLDAxNDQ0Hyc5PTgyPC4zNDL/2wBDAQkJCQwLDBgNDRgyIRwhMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjL/wAARCAAQABADASIAAhEBAxEB/8QAFQABAQAAAAAAAAAAAAAAAAAAAAf/xAAZEAADAAMAAAAAAAAAAAAAAAABAhEAEmH/xAAVAQEBAAAAAAAAAAAAAAAAAAAFBv/EABQRAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwEAAhEDEQA/AKaWJq6NZzAYiLo1nMFSK27WcwFJjbtZzIs8/9k="

    bgImageUrl = ""

    brickImage
    brickImageDimension

    bgImg
    bgImageDimension
    bgImage

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

    constructor(htmlCanvas, image_size, bgImageUrl) {
        this.bgImageUrl = bgImageUrl
        htmlCanvas.width = image_size
        htmlCanvas.height = image_size

        this.image = ti.Vector.field(4, ti.f32, [image_size, image_size]);
        this.raw_bg_image = ti.Vector.field(4, ti.f32, [image_size, image_size]);

        ti.addToKernelScope({
            image: this.image,
            raw_bg_image: this.raw_bg_image,
            img_size: image_size,
            plot_line: this.plotLine(),
            fill_rectangle: this.fillRectangle()
        })

        this.canvas = new ti.Canvas(htmlCanvas)

        this.brickImg = document.createElement("img")
        this.brickImg.id = "brickImage"
        this.brickImg.src = this.brickImageUrl

        this.bgImg = document.createElement("img")
        this.bgImg.id = "bgImage"
        this.bgImg.src = this.bgImageUrl

        // document.body.appendChild(this.bgImg)
    }

    async loadResources() {
        let promiseBrick = new Promise((resolve, reject) => {
            this.brickImg.onload = () => {
                this.hiddenCanvas = document.createElement("canvas")
                this.hiddenCanvas.style.width = this.brickImg.width + "px"
                this.hiddenCanvas.style.height = this.brickImg.height + "px"
                this.hiddenCanvas.width = this.brickImg.width
                this.hiddenCanvas.height = this.brickImg.height

                // document.body.appendChild(this.hiddenCanvas)

                let context = this.hiddenCanvas.getContext("2d")
                context.drawImage(this.brickImg, 0, 0)

                this.brickImage = ti.Vector.field(4, ti.i32, [this.brickImg.width, this.brickImg.height])
                let brickImageData = context.getImageData(0, 0, this.brickImg.width, this.brickImg.height)

                this.brickImage.fromArray1D(brickImageData.data)

                let brickImageSize = [this.brickImg.width, this.brickImg.height]
                ti.addToKernelScope({
                    brickImage: this.brickImage,
                    brickImageSize: brickImageSize,
                    renderBrick: this.renderBrick()
                })

                resolve(brickImageSize)
            }
        })

        let promiseBg = new Promise((resolve, reject) => {
            this.bgImg.onload = () => {
                this.hiddenCanvas = document.createElement("canvas")
                this.hiddenCanvas.style.width = this.bgImg.width + "px"
                this.hiddenCanvas.style.height = this.bgImg.height + "px"
                this.hiddenCanvas.width = this.bgImg.width
                this.hiddenCanvas.height = this.bgImg.height

                let context = this.hiddenCanvas.getContext("2d")
                context.fillStyle = "rgb(25, 39, 77)"
                context.fillRect(0, 0, this.bgImg.width, this.bgImg.height)
                context.drawImage(this.bgImg, 0, 0)

                // document.body.appendChild(this.hiddenCanvas)

                let bgImageData = context.getImageData(0, 0, this.bgImg.width, this.bgImg.height)

                this.bgImage = ti.Vector.field(4, ti.i32, [this.bgImg.width, this.bgImg.height])
                this.bgImage.fromArray1D(bgImageData.data)

                let bgImageSize = [this.bgImg.width, this.bgImg.height]
                ti.addToKernelScope({
                    bgImage: this.bgImage,
                    bgImageSize: bgImageSize
                })

                resolve(bgImageSize)
            }
        })

        return Promise.all([promiseBg, promiseBrick])
    }

    renderBrick() {
        return (img_coordinate, baseColor) => {
            let grid_diff = [dx * img_size, dx * img_size] * 2
            for (let i of range(grid_diff[0])) {
                for (let j of range(grid_diff[1])) {
                    let brick_image_coordinate = img_coordinate + [i, -j]

                    let texture_i = i32(f32(i) / f32(grid_diff[0]) * f32(brickImageSize[0]))
                    let texture_j = i32(f32(j) / f32(grid_diff[1]) * f32(brickImageSize[1]))

                    image[brick_image_coordinate] = brickImage[texture_i, texture_j] / 255.0 * baseColor
                }
            }
        }
    }

    render() {
        if (this.internalRenderKernel == null) {
            this.internalRenderKernel = ti.kernel(() => {

                for (let I of ndrange(img_size, img_size)) {
                    // image[I] = [0.067, 0.184, 0.255, 1.0];
                    //image[I] = [25 / 255, 39 / 255, 77 / 255, 1.0];

                    let texture_coordinate = f32(I) / f32(img_size)

                    let whratio = bgImageSize[1] / bgImageSize[0]
                    let originalBgImageCoordinate = [texture_coordinate[0] * bgImageSize[0], texture_coordinate[1] * bgImageSize[1]]
                    image[I] = [25 / 255, 39 / 255, 77 / 255, 1.0];
                    raw_bg_image[I] = image[I]


                    let originalConvertedBgImageCoordinate = [bgImageSize[1] - originalBgImageCoordinate[1], originalBgImageCoordinate[0] * whratio]
                    let convertedBgImageCoordinate = i32(originalConvertedBgImageCoordinate)
                    if (convertedBgImageCoordinate[0] > 0 && convertedBgImageCoordinate[0] <= bgImageSize[1] && convertedBgImageCoordinate[1] > 0 && convertedBgImageCoordinate[1] <= bgImageSize[0] ) {
                        // nearest
                        let pixelColor = bgImage[convertedBgImageCoordinate]
                        image[I] = pixelColor / 255.0
                        raw_bg_image[I] = image[I]
                    }
                }

                // Draw particles
                for (let i of range(n_particles)) {
                    if (active[i] == 0)
                        continue

                    let pos = x[i];
                    let ipos = i32(pos * img_size)

                    let guessed_original_position = x[i] - v[i] * dt * 50.0
                    let guessed_ipos = i32(guessed_original_position * img_size)

                    if (material[i] == 0) { // Water.
                        for (let x_i of ti.static(ti.range(5))) {
                            for (let y_j of ti.static(ti.range(5))) {
                                // if (!((x_i == 0 && y_j == 0)  // Draw 4 neighbours.
                                //     || (x_i == 0 && y_j == 2)
                                //     || (x_i == 2 && y_j == 0)
                                //     || (x_i == 2 && y_j == 2)
                                // ))
                                {
                                    let xoffset = x_i - 2
                                    let yoffset = y_j - 2

                                    let IPos = ipos + [xoffset, yoffset]
                                    let guessedIPos = guessed_ipos + [xoffset, yoffset]
                                    if (IPos[0] > 0 && IPos[1] > 0){
                                        // image[IPos] = particle_color[i];
                                        // image[IPos] = [0.0, 1.0, 1.0, 1.0] * 2.0 ;
                                        image[IPos] = [0.0, 1.0, 1.0, 1.0] * 1.5 ;
                                        image[IPos] = image[IPos] * raw_bg_image[guessedIPos]
                                    }
                                }
                            }
                        }
                    } else {
                        if (ipos[0] >= 0 && ipos[1] >= 0){
                            if(material[i] == 1){
                                for (let x_i of ti.static(ti.range(3))) {
                                    for (let y_j of ti.static(ti.range(3))) {
                                        // if (!((x_i == 0 && y_j == 0)  // Draw 4 neighbours.
                                        //     || (x_i == 0 && y_j == 2)
                                        //     || (x_i == 2 && y_j == 0)
                                        //     || (x_i == 2 && y_j == 2)
                                        // ))
                                        {
                                            let xoffset = x_i - 2
                                            let yoffset = y_j - 2

                                            let IPos = ipos + [xoffset, yoffset]
                                            if (IPos[0] > 0 && IPos[1] > 0) {
                                                image[IPos] = particle_color[i];
                                            }
                                        }
                                    }
                                }
                            }
                            image[ipos] = particle_color[i];
                        }
                    }
                }

                // Draw bricks
                for (let grid_x of range(n_grid)) {
                    for (let grid_y of range(n_grid)) {
                        let img_coordinate = i32([grid_x * dx * img_size, grid_y * dx * img_size])
                        if (grid_material[grid_x, grid_y] == 2) {
                            if (grid_material[grid_x + 1, grid_y - 1] == 2) {
                                renderBrick(img_coordinate, [0.7, 0.25, 0.17, 1.0])
                            }
                            image[img_coordinate] = [1.0, 0.0, 0.0, 1.0]
                        } else if (grid_material[grid_x, grid_y] == 1) {

                            if (grid_material[grid_x + 1, grid_y - 1] == 1) {
                                renderBrick(img_coordinate, [1.0, 1.0, 1.0, 1.0])
                            }

                            image[img_coordinate] = [0.0, 1.0, 0.0, 1.0]
                        }
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