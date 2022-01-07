export default class BasePage {
    loading = true;

    init() {
        const msgUpdate = 'New Update available! Please, reload the webapp to see the latest changes.';

        this._isUpdateAvailable()
            .then(isAvailable => {
                if (isAvailable)
                    this.showNotification(msgUpdate);
            });
    }

    showContent() {
        if (this.loading) {
            this.loading = false;
            document.getElementById('content').style.display = 'block';
            document.getElementById('app-spinner').style.display = 'none';
        }
    }

    showLoading() {
        if (!this.loading) {
            this.loading = true;
            document.getElementById('content').style.display = 'none';
            document.getElementById('app-spinner').style.display = 'block';
        }
    }

    showNotification(text) {
        document.getElementById('notification').style.display = 'block';
        document.getElementById('notification-text').innerText = text;
    }

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
