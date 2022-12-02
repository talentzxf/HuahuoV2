const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
    mode:"development",
    entry: {
        app: './src/index.js',
    },
    devtool: 'inline-source-map',
    devServer: {
        static: './dist',
        hot: true,
        host:"127.0.0.1",
        open:["http://127.0.0.1:8080"]
    },
    plugins: [
        new HtmlWebpackPlugin({
            title: 'Hot Module Replacement',
            template:"src/index.ejs"
        }),
    ],
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: 'ts-loader',
                exclude: /node_modules/,
            },
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