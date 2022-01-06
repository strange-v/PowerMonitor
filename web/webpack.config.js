const path = require('path'),
    fs = require('fs'),
	ini = require('ini'),
	webpack = require("webpack"),
	TerserPlugin = require('terser-webpack-plugin'),
	CopyPlugin = require("copy-webpack-plugin"),
	CompressionPlugin = require('compression-webpack-plugin'),
	MiniCssExtractPlugin = require('mini-css-extract-plugin'),
	RemovePlugin = require('remove-files-webpack-plugin');

const platformio = ini.parse(fs.readFileSync('../platformio.ini', 'utf-8'));
const moduleHost = platformio['env:REMOTE'].upload_port;
const mode = process.argv[process.argv.indexOf('--mode') + 1];
const isProduction = mode === 'production';
const dstProd = '../data';
const dstDev = 'public/power';
const dstPathFull = path.join(__dirname, isProduction ? dstProd : dstDev);
const hasCompressedCopy = (filePath) => {
    const file = path.parse(filePath);
    return file.ext !== '.gz' && fs.existsSync(path.join(file.dir, `${file.base}.gz`));
};


const config = {
    entry: {
        main: './src/index.js'
    },
    mode: mode,
    output: {
        path: dstPathFull,
        publicPath: '/power/',
        filename: '[name].js',
        hashDigestLength: 8
    },
    optimization: {
        minimize: isProduction,
        minimizer: [
            new TerserPlugin({ parallel: true })
        ]
    },
    module: {
        rules: [
            {
                test: /\.(svg)$/,
                type: 'asset/resource',
                generator: {
                    filename: 'images/[hash][ext][query]'
                }
            },
            {
                test: /\.s[ac]ss$/i,
                use: [
                    MiniCssExtractPlugin.loader,
                    'css-loader',
                    'sass-loader'
                ],
            },
        ],
    },
    plugins: [
        new CopyPlugin({
            patterns: [
                { from: 'src/statics', to: dstPathFull },
                { from: '*.html', to: dstPathFull, context: 'src/' },
            ],
        }),
        new CompressionPlugin,
        new MiniCssExtractPlugin,
    ],
};

if (isProduction) {
    config.plugins.push(new RemovePlugin({
        before: {
            root: dstPathFull,
            test: [{
                folder: '.',
                recursive: true,
                method: () => true
            }]
        },
        after: {
            root: dstPathFull,
            test: [
                {
                    folder: '.',
                    method: hasCompressedCopy
                },
                {
                    folder: './images',
                    method: hasCompressedCopy
                }
            ]
        }
    }));
} else {
    config.watch = true;
    config.devtool = 'source-map';
    config.devServer = {
        port: 9000,
        hot: true,
        proxy: {
            '/power/ws': {
                target: `ws://${moduleHost}`,
                ws: true
            },
            '/power/api': {
                target: `ws://${moduleHost}`
            },
        },
    };
}

module.exports = config;
