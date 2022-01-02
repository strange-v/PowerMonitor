const path = require('path');
const express = require('express');
const { createProxyMiddleware } = require('http-proxy-middleware');

const app = new express();

app.use('/power/ws', createProxyMiddleware({
    target: 'http://192.168.0.131',
    changeOrigin: true,
    ws: true
}));
app.get('*', (req, res) => {
    let file = 'index.html';
    if (req.path != '/power/')
        file = req.path.substr(7);

    res.sendFile(path.resolve(__dirname, '../data', file));
});


app.listen(3000, () => {
    console.log('App listening on port 3000')
});