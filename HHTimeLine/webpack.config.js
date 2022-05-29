const path = require("path");
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
    mode: 'development',
    entry: "./src/index.js",
    output: {
        path: path.resolve(__dirname, "dist"),
        // filename: "[name].bundle.js",
        filename: "hhtimeline.bundle.js",
        library: {
            name: "hhtimeline",
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
    plugins: [
        new HtmlWebpackPlugin({
            title: 'Development',
            template: 'src/index.ejs',
            // inject: false
        }),
    ]
};