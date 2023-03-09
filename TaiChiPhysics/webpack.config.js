const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

let gameName = "LoveMeltHeart"
// let gameName = "LearnWebGPU"
module.exports = {
    mode: "development",
    entry: {
        app: './src/' + gameName + '/index.js',
    },
    devtool: 'inline-source-map',
    devServer: {
        static: './dist',
        hot: true,
        host: "127.0.0.1",
        port: "8181",
        open: ["http://127.0.0.1:8181"]
    },
    plugins: [
        new HtmlWebpackPlugin({
            title: 'Hot Module Replacement',
            template: "src/" + gameName + "/index.ejs"
        }),
    ],
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: 'ts-loader',
                exclude: /node_modules/,
            },
            {
                test: /\.(gif|png|jpe?g)$/,
                type: "asset/resource",
                use: [
                    {
                        loader: "file-loader",
                        options: {
                            name: '[name].[ext]',
                            outputPath: 'static/img',
                        }
                    }
                ]
            }
        ],
    },
    resolve: {
        extensions: ['.tsx', '.ts', '.js'],
    },
    output: {
        filename: '[name].bundle.js',
        path: path.resolve(__dirname, 'dist'),
        clean: true,
    },
};