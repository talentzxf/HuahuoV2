const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require("copy-webpack-plugin");

module.exports = {
    mode:"development",
    entry: ['./src/index.ts'],
    output: {
        filename: 'main.js',
        path: path.resolve(__dirname, 'dist'),
    },
    resolve:{
        extensions: ['.tsx', '.ts', '.js']
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
    plugins: [
        new CopyPlugin({
            patterns: [
                { from:"../HuaHuoEngineV2/emcmake/HuaHuoEngineV2.wasm", to:"wasm"},
                { from:"../HuaHuoEngineV2/emcmake/HuaHuoEngineV2.js", to:"wasm"}
            ]
        }),
        new HtmlWebpackPlugin({
            title: 'Development',
            template: 'src/index.ejs',
            // inject: false
        }),
    ]
};