const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require("copy-webpack-plugin");
const FileManagerPlugin = require("filemanager-webpack-plugin")
// const MiniCssExtractPlugin = require('mini-css-extract-plugin')

let moduleExports = (env) => {

    let propertyFile = "./conf/hhide.default.properties"
    let mode = "development"
    if (env.production) {
        propertyFile = "./conf/hhide.prod.properties"
        mode = "production"
    }

    let destinationPath = path.resolve(__dirname, 'dist')
    let destinationPropertyFile = destinationPath + "\\hhide.properties"

    let apiSource = "../HuaHuoSwagger/build/swagger-code-huahuo_backend_api"
    let destinationApiFolder = destinationPath + "\\" + "clientApi"

    return {
        mode: mode,
        entry: ['./src/index.js'],
        devtool: 'inline-source-map',
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
                    test: /\.(scss)$/,
                    use: [
                        {
                            // Adds CSS to the DOM by injecting a `<style>` tag
                            loader: 'style-loader'
                            // // Extracts CSS for each JS file that includes CSS
                            // loader: miniCssExtractPlugin.loader
                        },
                        {
                            // Interprets `@import` and `url()` like `import/require()` and will resolve them
                            loader: 'css-loader'
                        },
                        {
                            // Loader for webpack to process CSS with PostCSS
                            loader: 'postcss-loader',
                            options: {
                                postcssOptions: {
                                    plugins: () => [
                                        autoprefixer
                                    ]
                                }
                            }
                        },
                        {
                            // Loads a SASS/SCSS file and compiles it to CSS
                            loader: 'sass-loader'
                        }
                    ]
                },
                {
                    test: /\.tsx?$/,
                    use: 'ts-loader',
                    exclude: /node_modules/,
                },

                {
                    test: /\.css$/i,
                    use: ["style-loader", "css-loader", "postcss-loader"]
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
                directory: destinationPath
            },
            compress: true,
            port: 8989,
            client: {
                overlay: false
            }
        },
        watchOptions: {
            ignored: ['**/node_modules/**', "**/dist/**"]
        },
        plugins: [
            new CopyPlugin({
                patterns: [
                    {from: "../HuahuoEngines/HuaHuoEngineV2/emcmake/HuaHuoEngineV2.wasm", to: "wasm"},
                    {from: "../HuahuoEngines/HuaHuoEngineV2/emcmake/HuaHuoEngineV2.js", to: "wasm"},
                    {from: "./svgs", to: "svgs"},
                    {from: "./src/i18n", to: "i18n"},
                    {from: "./src/test_lgraph.html", to: "test_lgraph.html"},
                    {from: "./static", to: "static"},
                    {from: "../Libs/gif.js/dist", to:"static/gif.js/dist"},
                    {from: "../Libs/litegraph.js/css", to: "static"}
                ],
            }),
            new FileManagerPlugin({
                events: {
                    onStart: [{
                        delete: [{
                            source: destinationPropertyFile,
                            options: {
                                force: true
                            }
                        }, {
                            source: destinationPropertyFile,
                            options: {
                                force: true
                            }
                        }],
                        copy: [{
                            source: propertyFile,
                            destination: destinationPropertyFile,
                            options: {
                                flat: false,
                                preserveTimestamps: true,
                                overwrite: true,
                            }
                        }, {
                            source: apiSource,
                            destination: destinationApiFolder,
                            options: {
                                flat: false,
                                preserveTimestamps: true,
                                overwrite: true
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
            // new MiniCssExtractPlugin()
        ],
    };

}

module.exports = moduleExports