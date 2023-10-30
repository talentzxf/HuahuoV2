const path = require("path");
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = (env) => {
    let mode = "development"
    if(env.production)
        mode = "production"

    return {
        mode: mode,
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
        module: {
            rules: [
                {
                    test: /\.tsx?$/,
                    use: 'ts-loader',
                    exclude: /node_modules/,
                },
            ]
        },
        externals: {
            "hhcommoncomponents": "hhcommoncomponents",
            "hhenginejs": "hhenginejs"
        },
        resolve: {
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
        ],
    }
};