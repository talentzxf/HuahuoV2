const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require("copy-webpack-plugin");
const FileManagerPlugin = require("filemanager-webpack-plugin")

module.exports = (env) => {
    let propertyFile = "./conf/hhplayer.default.properties"
    if(env.production){
        propertyFile = "./conf/hhplayer.prod.properties"
    }

    let destinationPath = path.resolve(__dirname, 'dist')

    let destinationPropertyFile = destinationPath + "/hhplayer.properties"

    return {
        mode: "development",
        entry: ['./src/index.js'],
        output: {
            filename: 'main.js',
            path: path.resolve(__dirname, 'dist'),
        },
        resolve: {
            extensions: ['.tsx', '.ts', '.js']
        },
        module: {
            rules: [
                {
                    test: /\.tsx?$/,
                    use: 'ts-loader',
                    exclude: /node_modules/,
                },
                {
                    test: /\.css$/i,
                    use: ["style-loader", "css-loader"]
                },
                {
                    test: /\.properties$/i,
                    use: [
                        {
                            loader: 'properties-file/webpack-loader',
                        },
                    ],
                },
            ]
        },
        devServer: {
            static: {
                directory: path.join(__dirname, "dist"),
            },
            compress: true,
            port: 8989,
        },
        watchOptions:{
            ignored:[
                destinationPath, destinationPath + "/*"
            ]
        },
        plugins: [
            new CopyPlugin({
                patterns: [
                    {from: "../HuaHuoEngineV2/emcmake/HuaHuoEngineV2.wasm", to: "wasm"},
                    {from: "../HuaHuoEngineV2/emcmake/HuaHuoEngineV2.js", to: "wasm"},
                ]
            }),
            new FileManagerPlugin({
                events:{
                    onStart: [{
                        delete:[destinationPropertyFile],
                        copy: [{
                                source: propertyFile,
                                destination: destinationPropertyFile,
                                options:{
                                    overwrite: true,
                                }
                            }]
                    }]
                }
            }),
            new HtmlWebpackPlugin({
                title: 'Development',
                template: 'src/index.ejs',
                // inject: false
            }),
        ]
    };
}