// webpack.config.js
const path = require('path');

module.exports = {
    entry: './src/index.ts', // 入口文件路径
    output: {
        filename: 'bundle.js', // 输出文件名
        path: path.resolve(__dirname, 'dist'), // 输出目录路径
    },
    resolve: {
        extensions: ['.ts', '.js'], // 解析文件扩展名
    },
    module: {
        rules: [
            {
                test: /\.ts$/, // 匹配以 .ts 结尾的文件
                use: 'ts-loader', // 使用 ts-loader 来处理 TypeScript 文件
                exclude: /node_modules/, // 排除 node_modules 文件夹
            },
        ],
    },
};