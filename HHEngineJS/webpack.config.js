const path = require("path");
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyPlugin = require("copy-webpack-plugin")

let webpackConfig = {
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
    resolve: {
        extensions: ['.tsx', '.ts', '.js']
    },
    externals: {
        "hhcommoncomponents": "hhcommoncomponents"
    },
    devServer: {
        static: {
            directory: path.join(__dirname, "dist"),
        },
        compress: true,
        port: 9393,
        client: {
            overlay: false
        }
    },
    plugins: [
        new CopyPlugin({
            patterns: [
                {from: "../HuahuoEngines/HuaHuoEngineV2/emcmake/HuaHuoEngineV2.wasm", to: "wasm"},
                {from: "../HuahuoEngines/HuaHuoEngineV2/emcmake/HuaHuoEngineV2.js", to: "wasm"},
                {from: "../HuahuoEngines/HuaHuoEngineV2/emcmake/engine.properties", to: "conf"},
                {from: "./test/test.js", to: "./"},
            ],
        }),
        new HtmlWebpackPlugin({
            title: 'Development',
            template: 'test/test.ejs',
            // inject: false
        })
    ]
};


function setupWebpack(env) {
    if (env["goal"] == "local") {
        webpackConfig.entry = ["./src/index.js", "./test/test.js"]
    }

    if (env.production) {
        webpackConfig.mode = "production"
    }

    return webpackConfig
}

module.exports = setupWebpack