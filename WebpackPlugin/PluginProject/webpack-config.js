const path = require('path')
module.exports = {
    // mode: "production",
    mode: "development",
    entry: "./src/index.ts",
    output: {
        filename: "huahuoplugin.js",
        path: path.resolve(__dirname, "dist"),
        libraryTarget: 'commonjs2' // 输出为 CommonJS2 格式
    },
    resolve: {
        extensions:[".ts", ".js"]
    },
    module: {
        rules: [
            {
                test: /\.ts$/, // 匹配所有.ts文件
                exclude: /node_modules/, // 排除node_modules文件夹
                use: {
                    loader: 'babel-loader'
                }
            }
        ]
    },
    plugins: []
}