const path = require("path");
const HtmlWebpackPlugin = require('html-webpack-plugin');

let webpackExport = {
    mode: 'development',
    entry: "./src/index.js",
    output: {
        path: path.resolve(__dirname, "dist"),
        // filename: "[name].bundle.js",
        filename: "hhpanel.bundle.js",
        library: {
            name: "hhpanel",
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
        ]
    },

    resolve: {
        extensions: ['.tsx', '.ts', '.js']
    },
    devServer: {
        static: {
            directory: path.join(__dirname, "dist"),
        },
        compress: true,
        port: 9090,
    },
    plugins: [
        new HtmlWebpackPlugin({
            title: 'Development',
            template: 'src/index.ejs',
            // inject: false
        }),
    ]
};

if (process.env.WEBPACK_DEV_SERVER) {
    webpackExport = Object.assign(webpackExport, {
        externals: {
            "hhcommoncomponents": "hhcommoncomponents"
        },
    })
}

module.exports = (env)=>{
    if(env.production){
        webpackExport.mode = "production"
    }
    return webpackExport
}