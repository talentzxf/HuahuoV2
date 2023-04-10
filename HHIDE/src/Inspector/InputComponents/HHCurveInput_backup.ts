// import {CustomElement} from "hhcommoncomponents";
// import {ContextMenu} from "hhcommoncomponents";
// import {Vector2} from "hhcommoncomponents";
//
// declare class KeyFrameCurvePoint {
//     GetValue(): number
//
//     GetFrameId(): number
//
//     GetHandleIn(): Vector2
//
//     GetHandleOut(): Vector2
// }
//
// class ViewPort {
//     canvasWidth
//     canvasHeight
//
//     viewWidth
//     viewHeight
//
//     viewXMin
//     viewYMin
//     viewXMax
//     viewYMax
//     leftDown
//
//     get viewXSpan() {
//         return this.viewXMax - this.viewXMin
//     }
//
//     get viewYSpan() {
//         return this.viewYMax - this.viewYMin
//     }
//
//     viewToCanvas(x, y) {
//         let xScale = this.viewWidth / this.viewXSpan
//         let yScale = this.viewHeight / this.viewYSpan
//
//         let canvasX = this.leftDown[0] + (x - this.viewXMin) * xScale
//         let canvasY = this.leftDown[1] - (y - this.viewYMin) * yScale
//
//         return [canvasX, canvasY]
//     }
// }
//
// class ViewPoint {
//     frameId: number
//     value: number
//     handleIn: number[]
//     handleOut: number[]
//     radius: number
//     viewPort: ViewPort
//
//     // Cache the canvasPosition to avoid multiple calculations.
//     canvasPosition: number[]
//
//     constructor(point: KeyFrameCurvePoint, radius: number, viewPort: ViewPort) {
//         this.frameId = point.GetFrameId()
//         this.value = point.GetValue()
//         this.handleIn = point.GetHandleIn()
//         this.handleOut = point.GetHandleOut()
//         this.radius = radius
//         this.viewPort = viewPort
//
//         this.canvasPosition = this.viewPort.viewToCanvas(this.frameId, this.value)
//     }
//
//     setHandleIn(x, y){
//         // TODO: Save in Cpp side.
//         this.handleIn = new Vector2(x, y)
//     }
//
//     setHandleOut(x, y){
//         // TODO: Save in Cpp side.
//         this.handleOut = new Vector2(x, y)
//     }
//
//     contains(canvasX, canvasY) {
//         let distSquare = Math.pow(canvasX - this.canvasPosition[0], 2) + Math.pow(canvasY - this.canvasPosition[1], 2)
//         if (distSquare <= this.radius * this.radius)
//             return true
//         return false
//     }
// }
//
// @CustomElement({
//     selector: "hh-curve-input"
// })
// class HHCurveInput extends HTMLElement {
//     canvas: HTMLCanvasElement
//     ctx: CanvasRenderingContext2D
//
//     infoPrompt: HTMLDivElement
//
//     keyFrameCurveGetter: Function
//     viewPort: ViewPort = new ViewPort()
//
//     viewPoints: ViewPoint[]
//
//     infoPromptContextMenu: ContextMenu = new ContextMenu
//
//     constructor(keyFrameCurveGetter) {
//         super();
//
//         this.keyFrameCurveGetter = keyFrameCurveGetter
//
//         this.canvas = document.createElement("canvas")
//         this.ctx = this.canvas.getContext("2d")
//
//         this.infoPrompt = document.createElement("div")
//         this.infoPrompt.style.position = "absolute"
//         this.infoPrompt.className = "tooltip-text"
//
//         this.appendChild(this.infoPrompt)
//         this.hideInfoPrompt()
//
//         this.canvas.onmousemove = this.onMouseMove.bind(this)
//         this.canvas.onmousedown = this.onMouseDown.bind(this)
//
//         this.appendChild(this.canvas)
//
//         // Make the position as relative, so infoPrompt can regard HHCurveInput as it's nearest positioned ancestor. https://www.w3.org/TR/css-position-3/
//         this.style.position = "relative"
//     }
//
//     hideInfoPrompt() {
//         this.infoPrompt.style.display = "none"
//     }
//
//     showInfoPrompt() {
//         this.infoPrompt.style.display = "block"
//     }
//
//     connectedCallback() {
//         this.refresh()
//     }
//
//     hitSomething(canvasX, canvasY, hitCallback, nohitCallback: Function = null) {
//         let somethingHitMouse = false;
//         for (let viewPoint of this.viewPoints) {
//             if (viewPoint.contains(canvasX, canvasY)) {
//                 hitCallback(viewPoint)
//                 somethingHitMouse = true;
//             }
//         }
//
//         if (!somethingHitMouse) {
//             if (nohitCallback)
//                 nohitCallback()
//         }
//     }
//
//     onMouseMove(e: MouseEvent) {
//         let _this = this
//         this.hitSomething(e.offsetX, e.offsetY, (viewPoint: ViewPoint) => {
//             // TODO: i18n
//             this.infoPrompt.innerText = i18n.t("inspector.CurveInputPrompt", {
//                 frameId: viewPoint.frameId,
//                 value: viewPoint.value
//             })
//
//             _this.showInfoPrompt()
//
//             let offsetX = e.offsetX
//             let offsetY = e.offsetY
//
//             // Check if over the current right border.
//             let toolTipBorder = this.infoPrompt.getBoundingClientRect()
//             let currentInputBorder = this.getBoundingClientRect()
//             if (offsetX + toolTipBorder.width > currentInputBorder.width) {
//                 offsetX -= toolTipBorder.width;
//             }
//
//             this.infoPrompt.style.left = offsetX + "px"
//             this.infoPrompt.style.top = offsetY + "px"
//         }, () => {
//             _this.hideInfoPrompt()
//         })
//     }
//
//     onMouseDown(evt: MouseEvent) {
//         let _this = this
//         this.hitSomething(evt.offsetX, evt.offsetY, (viewPoint: ViewPoint) => {
//             _this.infoPromptContextMenu.setItems([
//                 {
//                     itemName: i18n.t("inspector.Smooth"),
//                     onclick: () => {
//                         // Smooth of Bezier is complicated. Write a very simple one here.
//                         // TODO: https://www.particleincell.com/2012/bezier-splines/
//
//                         viewPoint.setHandleIn(1.0, 1.0)
//                         viewPoint.setHandleOut(-1.0, -1.0)
//
//                     }
//                 },
//                 {
//                     itemName: i18n.t("inspector.Sharpen"),
//                     onclick: () => {
//
//                     }
//                 },
//             ])
//
//             _this.infoPromptContextMenu.onContextMenu(evt)
//         })
//     }
//
//     refresh() {
//         let curve = this.keyFrameCurveGetter()
//         if (curve == null)
//             return
//
//         let minValue = Number.MAX_VALUE
//         let maxValue = -Number.MAX_VALUE
//
//         let minFrameId = Number.MAX_VALUE
//         let maxFrameId = -Number.MAX_VALUE
//
//         let points = []
//         let totalPoints = curve.GetTotalPoints()
//         for (let pointIdx = 0; pointIdx < totalPoints; pointIdx++) {
//             let curvePoint = curve.GetKeyFrameCurvePoint(pointIdx)
//             let value = curvePoint.GetValue()
//             let frameId = curvePoint.GetFrameId() + 1 // In Cpp side, frameId starts from 0. But when shown, frameId starts from 1.
//
//             minValue = Math.min(minValue, value)
//             maxValue = Math.max(maxValue, value)
//             minFrameId = Math.min(minFrameId, frameId)
//             maxFrameId = Math.max(maxFrameId, frameId)
//
//             points.push(curvePoint)
//         }
//
//         // Clear background
//         this.ctx.fillStyle = "lightgray"
//         this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height)
//
//         // Add some offset to avoid 0/0
//         if (minFrameId == maxFrameId) {
//             maxFrameId = minFrameId + 1
//         }
//
//         if (minValue == maxValue) {
//             maxValue = minValue + 1
//         }
//
//         // Draw the coordinate
//         this.viewPort.canvasWidth = this.canvas.width
//         this.viewPort.canvasHeight = this.canvas.height
//         this.viewPort.viewWidth = 0.8 * this.canvas.width
//         this.viewPort.viewHeight = 0.8 * this.canvas.height
//         this.viewPort.viewXMin = minFrameId
//         this.viewPort.viewXMax = maxFrameId
//         this.viewPort.viewYMin = minValue
//         this.viewPort.viewYMax = maxValue
//         this.viewPort.leftDown = [0.05 * this.canvas.width, 0.9 * this.canvas.height]
//
//         let canvasOrigin = this.viewPort.viewToCanvas(minFrameId, minValue)
//
//         let xMax = this.viewPort.viewToCanvas(maxFrameId, minValue)
//         let yMax = this.viewPort.viewToCanvas(minFrameId, maxValue)
//
//         this.ctx.beginPath()
//         this.ctx.moveTo(canvasOrigin[0], canvasOrigin[1])
//         this.ctx.lineTo(xMax[0], xMax[1])
//         this.ctx.moveTo(canvasOrigin[0], canvasOrigin[1])
//         this.ctx.lineTo(yMax[0], yMax[1])
//
//         this.ctx.strokeStyle = "green"
//         this.ctx.stroke()
//
//         this.ctx.font = "10px serif"
//         this.ctx.fillStyle = "black"
//
//         this.viewPoints = []
//
//         let radius = 3
//
//         // Draw the key points
//         for (let point of points) {
//             let frameId = point.GetFrameId() + 1
//             let value = point.GetValue()
//
//             this.ctx.beginPath()
//             let canvasPoint = this.viewPort.viewToCanvas(frameId, value)
//
//             this.ctx.arc(canvasPoint[0], canvasPoint[1], radius, 0, 2 * Math.PI, true)
//             this.ctx.fillStyle = "blue"
//             this.ctx.fill()
//
//             this.viewPoints.push(new ViewPoint(point, radius, this.viewPort))
//
//             // Draw X-Axis text.
//             let position = this.viewPort.viewToCanvas(frameId, minValue)
//             let string = String(frameId)
//             let textRect = this.ctx.measureText(string)
//
//             let actualHeight = textRect.actualBoundingBoxAscent + textRect.actualBoundingBoxDescent;
//             this.ctx.fillText(string, position[0] - textRect.width / 2.0, position[1] + actualHeight + 5)
//
//             // Draw Y-Axis text.
//             position = this.viewPort.viewToCanvas(minFrameId, value)
//             string = String(value)
//             textRect = this.ctx.measureText(string)
//             actualHeight = textRect.actualBoundingBoxAscent + textRect.actualBoundingBoxDescent;
//             this.ctx.fillText(string, position[0] - textRect.width - 5, position[1] + actualHeight / 2.0)
//         }
//     }
//
// }
//
// export {HHCurveInput}