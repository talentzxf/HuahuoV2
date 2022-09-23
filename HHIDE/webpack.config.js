const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require("copy-webpack-plugin");
const FileManagerPlugin = require("filemanager-webpack-plugin")

let moduleExports = (env)=> {

    let propertyFile = "./conf/hhide.default.properties"
    if(env.production){
        propertyFile = "./conf/hhide.prod.properties"
    }

    let destinationPath = path.resolve(__dirname, 'dist')
    let destinationPropertyFile = destinationPath + "\\hhide.properties"

    return {
        mode: "development",
        entry: ['./src/index.js'],
        output: {
            filename: 'main.js',
            path: destinationPath
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
                    {from: "./src/i18n", to: "i18n"},
                ],
            }),
            new FileManagerPlugin({
                events:{
                    onStart: [{
                        delete:[{
                            source: destinationPropertyFile,
                            options:{
                                force: true
                            }
                        }],
                        copy: [{
                            source: propertyFile,
                            destination: destinationPropertyFile,
                            options:{
                                flat: false,
                                preserveTimestamps: true,
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

module.exports = moduleExports