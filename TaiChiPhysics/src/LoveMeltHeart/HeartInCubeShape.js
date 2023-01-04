import {BaseShape} from "./Shapes";

class HeartInCubeShape extends BaseShape {
    center = [0.1, 0.1]
    size = 0.1

    imgUrl = "data:image/svg+xml,%3C%3Fxml version='1.0' encoding='iso-8859-1'%3F%3E%3C!-- Generator: Adobe Illustrator 16.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0) --%3E%3C!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'%3E%3Csvg version='1.1' id='Capa_1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' x='0px' y='0px' width='45.743px' height='45.743px' viewBox='0 0 45.743 45.743' style='enable-background:new 0 0 45.743 45.743;' xml:space='preserve'%3E%3Cg%3E%3Cpath d='M34.199,3.83c-3.944,0-7.428,1.98-9.51,4.997c0,0-0.703,1.052-1.818,1.052c-1.114,0-1.817-1.052-1.817-1.052 c-2.083-3.017-5.565-4.997-9.51-4.997C5.168,3.83,0,8.998,0,15.376c0,1.506,0.296,2.939,0.82,4.258 c3.234,10.042,17.698,21.848,22.051,22.279c4.354-0.431,18.816-12.237,22.052-22.279c0.524-1.318,0.82-2.752,0.82-4.258 C45.743,8.998,40.575,3.83,34.199,3.83z'/%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3Cg%3E%3C/g%3E%3C/svg%3E%0A"

    hiddenCanvas
    img

    heartMask

    constructor(image_size) {
        super();
        this.img = document.createElement("img")
        this.img.src = this.imgUrl

        // document.body.appendChild(this.img)
    }

    async loadImage() {
        let promise = new Promise((resolve, reject) => {
            this.img.onload = () => {
                this.hiddenCanvas = document.createElement("canvas")
                this.hiddenCanvas.style.width = this.img.width + "px"
                this.hiddenCanvas.style.height = this.img.height + "px"
                this.hiddenCanvas.width = this.img.width
                this.hiddenCanvas.height = this.img.height
                let context = this.hiddenCanvas.getContext("2d")
                context.drawImage(this.img, 0, 0)
                // document.body.appendChild(this.hiddenCanvas)

                this.heartMask = ti.field(ti.i32, [this.img.width, this.img.height]);
                let imgData = context.getImageData(0, 0, this.img.width, this.img.height)
                for (let i = 0; i < this.img.width; i++) {
                    for (let j = 0; j < this.img.height; j++) {
                        this.heartMask.set([j, this.img.width - i], imgData.data[i * 4 * this.img.width + j * 4 + 3])
                    }
                }

                resolve([this.img.width, this.img.height])
            }
        })
        return promise
    }

    async addParametersToKernel() {
        super.addParametersToKernel();

        let image_size = await this.loadImage()

        ti.addToKernelScope({
            center: this.center,
            size: this.size,
            heartMask: this.heartMask,
            heartMaskSize: image_size
        })
    }

    nextPosition() {
        return (total, idx) => {
            let resultPosition = [
                ti.random() * size + center[0] - size / 2.0,
                ti.random() * size + center[1] - size / 2.0
            ]

            return resultPosition
        }
    }


    async reset(offsetX = 0.0, offsetY = 0.0) {
        await super.reset(offsetX, offsetY);

        // verify heart mask

        let testKernel = ti.kernel(
            //(posX, posY) =>
            (idx) =>
            {
                let position = x[i32(idx)]
                let returnColor = [0.0, 1.0, 1.0, 1.0]
                if (position[0] >= position[1]) {
                    returnColor = [1.0, 0.0, 0.0, 1.0]
                }
                return [position[0],position[1], returnColor[0],returnColor[1],returnColor[2]]
                //return unifiedOffsetLeftDown
                // return position
                //return [position, heartMask[heartCoordinate]]
            })

        testKernel(this.startIdx + 256).then((val) => {
            console.log("Result (0.1,0.1) is:" + val)
        })
    }

    rhoFunc(){
        return ()=>{
            return 1.0
        }
    }

    nextMaterial() {
        return (position, materialId) => {
            let leftDown = center - size / 2

            let offsetLeftDown = position - leftDown

            let unifiedOffsetLeftDown = offsetLeftDown / size // Map to [0.0, 0.0] --- [1.0, 1.0]

            // Map to heart map
            let heartCoordinate = i32(unifiedOffsetLeftDown * heartMaskSize)

            let returnMaterial = 1
            // if (heartMask[heartCoordinate] > 0) {
            //     returnMaterial = 1
            // }

            return returnMaterial
        }
    }

    colorFunc() {
        return (position, materialId) => {
            let leftDown = center - size / 2

            let offsetLeftDown = position - leftDown

            let unifiedOffsetLeftDown = offsetLeftDown / size // Map to [0.0, 0.0] --- [1.0, 1.0]

            // Map to heart map
            let heartCoordinate = i32(unifiedOffsetLeftDown * heartMaskSize)

            let returnColor = [1.0, 1.0, 1.0, 1.0]
            if (heartMask[heartCoordinate] > 0) {
                returnColor = [1.0, 0.0, 0.0, 1.0]
            }
            return returnColor
        }
    }
}

export {HeartInCubeShape}