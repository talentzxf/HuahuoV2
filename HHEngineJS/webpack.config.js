const path = require("path");
const TypescriptDeclarationPlugin = require('typescript-declaration-webpack-plugin');

module.exports = {
    mode: 'development',
    entry: "./src/index.js",
    output: {
        path: path.resolve(__dirname, "dist"),
        // filename: "[name].bundle.js",
        filename: "hhenginejs.bundle.js",
        library: {
            name: "hhenginejs",
            type: "umd"
        }
    },
    module:{
        rules: [
            {
                test: /\.tsx?$/,
                use: 'ts-loader',
                exclude: /node_modules/,
            },
            {
                test: /\.css$/i,
                use:["style-loader","css-loader"]
            }
        ]
    },
    resolve:{
        extensions: ['.tsx', '.ts', '.js']
    },
    devServer: {
        static: {
            directory: path.join(__dirname, "dist"),
        },
        compress: true,
        port: 9393,
    },
    // plugins:[
    //     new TypescriptDeclarationPlugin({
    //         out:'hhenginejs.bundle.d.ts'
    //     })
    // ]
};