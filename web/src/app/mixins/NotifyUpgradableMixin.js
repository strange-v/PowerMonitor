export default {
    checkForUpdate() {
        const msgUpdate = 'New Update available! Please, reload the webapp to see the latest changes.';
        
        this._isUpdateAvailable()
            .then(isAvailable => {
                if (isAvailable)
                    this._showNotification(msgUpdate);
            });
    },
    
    _showNotification(text) {
        document.getElementById('notification').style.display = 'block';
        document.getElementById('notification-text').innerText = text;
    },

    _isUpdateAvailable() {
        return new Promise(function (resolve, reject) {
            if ('serviceWorker' in navigator && ['localhost', '127'].indexOf(location.hostname) === -1) {
                navigator.serviceWorker.register('/power/service-worker.js')
                    .then(reg => {
                        reg.onupdatefound = () => {
                            const installingWorker = reg.installing;
                            installingWorker.onstatechange = () => {
                                switch (installingWorker.state) {
                                    case 'installed':
                                        if (navigator.serviceWorker.controller) {
                                            resolve(true);
                                        } else {
                                            resolve(false);
                                        }
                                        break;
                                }
                            };
                        };
                    })
                    .catch(err => console.error('[SW ERROR]', err));
            }
        });
    }
}