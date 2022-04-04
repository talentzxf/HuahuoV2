const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require("copy-webpack-plugin");

module.exports = {
    mode:"development",
    entry: './src/index.js',
    output: {
        filename: 'main.js',
        path: path.resolve(__dirname, 'dist'),
    },
    plugins: [
        new CopyPlugin({
            patterns: [
                { from:"../PersistentManager/emcmake/PersistentManager.wasm", to:"wasm"},
                { from:"../PersistentManager/emcmake/PersistentManager.js", to:"wasm"}
            ]
        }),
        new HtmlWebpackPlugin({
            title: 'Development',
            template: 'src/index.ejs',
            // inject: false
        }),
    ]
};