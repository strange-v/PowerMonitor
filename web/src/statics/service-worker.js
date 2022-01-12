const cacheName = '{{version}}';
const contentToCache = [
    '/power/',
    '/power/index.html',
    '/power/main.js',
    '/power/main.css',
    '/power/favicon.ico',
    '/power/favicon-16x16.png',
    '/power/favicon-32x32.png',
    '/power/icons-192.png',
    '/power/icons-512.png',
];

self.addEventListener('install', (e) => {
    console.log('[Service Worker] Install');
    e.waitUntil((async () => {
        const cache = await caches.open(cacheName);
        console.log('[Service Worker] Caching all: app shell and content');
        await cache.addAll(contentToCache);
    })());
});

self.addEventListener('fetch', (e) => {});
