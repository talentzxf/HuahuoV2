// TODO: Import SVG into the system (Might be complicated)
// import {BaseSolidShape} from "./BaseSolidShape";
// let shapeName = "BaseShape"
//
// class SVGShapeJS extends BaseSolidShape {
//     static createShape(rawObj) {
//         return new SVGShapeJS()
//     }
//
//     shapeURL: string
//
//     getShapeName(): string {
//         return shapeName;
//     }
//
//     setShapeURL(shapeURL) {
//         this.shapeURL = shapeURL
//     }
//
//     isSegmentSeletable(): boolean {
//         return false;
//     }
//
//     createShape() {
//         super.createShape()
//
//         let _this = this
//         let paperJs = this.getPaperJs()
//
//         let createShapePromiseResolver
//         let createShapePromise = new Promise( (resolve, reject)=>{
//             createShapePromiseResolver = resolve
//         })
//
//         paperJs["project"]["importSVG"](this.shapeURL,
//         {
//             expandShapes: true,
//             onLoad: function (item) {
//                 _this.paperShape = item
//                 _this.paperShape.applyMatrix = false
//                 // _this.paperShape.strokeColor = new paper.Color("black")
//                 // _this.paperShape.fillColor = new paper.Color("green")
//                 _this.paperShape.data.meta = this
//                 _this.paperShape.strokeColor = new paper.Color("black")
//                 _this.paperShape.fillColor = new paper.Color("black")
//
//                 // Recurisively set the meta.
//                 let shapeStack: Array<paper.Item> = new Array()
//                 shapeStack.push(_this.paperShape)
//                 while (shapeStack.length > 0) {
//                     let currentShape = shapeStack.pop()
//                     currentShape.data.meta = _this
//
//                     if (currentShape.children) {
//                         for (let child of currentShape.children) {
//                             shapeStack.push(child)
//                         }
//                     }
//                 }
//
//                 _this.afterCreateShape()
//
//                 _this.store()
//
//                 createShapePromiseResolver()
//             }
//             })
//
//         return createShapePromise
//     }
//
//     update(force: boolean = false) {
//         if (this.paperShape) {
//             super.update(force)
//         }
//     }
// }
//
// export {SVGShapeJS}