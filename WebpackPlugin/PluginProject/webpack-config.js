const path = require('path')
module.exports = {
    mode: "production",
    entry: "./src/index.ts",
    output: {
        filename: "huahuoplugin.js",
        path: path.resolve(__dirname, "dist")
    },
    resolve: {
        extensions:[".ts", ".js"]
    },
    module: {
        rules: [
            {
                test: /\.ts$/,
                use: 'ts-loader',
                exclude: /node_modules/,
            }
        ]
    },
    plugins: []
}