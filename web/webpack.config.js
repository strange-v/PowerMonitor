const path = require('path'),
    fs = require('fs'),
    ini = require('ini'),
    webpack = require("webpack"),
    TerserPlugin = require('terser-webpack-plugin'),
    CopyPlugin = require("copy-webpack-plugin"),
    CompressionPlugin = require('compression-webpack-plugin'),
    MiniCssExtractPlugin = require('mini-css-extract-plugin'),
    HtmlWebpackPlugin = require('html-webpack-plugin'),
    RemovePlugin = require('remove-files-webpack-plugin'),
    MiniSvgDataPlugin = require('mini-svg-data-uri'),
    package = require('./package.json'),
    crypto = require('crypto');

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
        main: './src/index.js',
    },
    mode: mode,
    resolve: {
        alias: {
            'app': path.resolve(__dirname, './src/app'),
        }
    },
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
                type: 'asset/inline',
                generator: {
                    dataUrl(content) {
                        return MiniSvgDataPlugin(content.toString());
                    }
                },
                use: {
                    loader: 'svgo-loader',
                    options: {
                        configFile: '../svgo.config.js'
                    }
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
            {
                test: /\.css$/i,
                use: [
                    MiniCssExtractPlugin.loader,
                    'css-loader',
                ],
            },
            {
                test: /\.html$/,
                use: 'html-loader'
            },
        ],
    },
    plugins: [
        new CopyPlugin({
            patterns: [
                {
                    from: 'src/statics',
                    to: dstPathFull,
                    transform(content, filePath) {
                        if (path.parse(filePath).base == 'service-worker.js') {
                            return content
                                .toString()
                                .replace('{{version}}', `${package.version}-${crypto.randomBytes(8).toString('hex')}`)
                        }
                        return content;
                    }
                },
            ],
        }),
        new CompressionPlugin,
        new MiniCssExtractPlugin,
        new HtmlWebpackPlugin({
            title: 'Power Monitor',
            templateContent: `
            <html>
                <head>
                    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1" />
                    <link rel="icon" type="image/png" sizes="32x32" href="/power/favicon-32x32.png">
                    <link rel="icon" type="image/png" sizes="16x16" href="/power/favicon-16x16.png">
                    <meta name="theme-color" content="#1f1f1f">
                    <link rel="manifest" href="/power/manifest.json" crossorigin="use-credentials">
                </head>
                <body>
                    <div id="app"></div>
                </body>
            </html>
            `
        }),
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
