const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require("copy-webpack-plugin");

module.exports = {
    mode:"development",
    entry: ['./src/index.js'],
    output: {
        filename: 'main.js',
        path: path.resolve(__dirname, 'dist'),
    },
    plugins: [
        new CopyPlugin({
            patterns: [
                { from:"../HuaHuoEngine/emcmake/HuaHuoEngine.wasm", to:"wasm"},
                { from:"../HuaHuoEngine/emcmake/HuaHuoEngine.js", to:"wasm"}
            ]
        }),
        new HtmlWebpackPlugin({
            title: 'Development',
            template: 'src/index.ejs',
            // inject: false
        }),
    ]
};