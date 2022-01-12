import LiveDataPage from "./app/pages/LiveDataPage";
import SettingsPage from "./app/pages/SettingsPage";
import Router from "./app/Router";
import "./main.scss";

class App {
    routes = {
        '/': LiveDataPage,
        '/settings': SettingsPage
    };

    init() {
        this._router = new Router;
        this._router.on('change', this._onRouteChange, this);

        this._router.init();
    }

    _onRouteChange(_, e) {
        if (this.routes[e.route] === undefined) {
            throw 'Implement 404';
        }

        if (this._view) {
            this._view.destroy();
            delete this._view;
        }

        this._view = new this.routes[e.route];
        this._view.init();
    }
}

const app = new App;

document.addEventListener("DOMContentLoaded", () => app.init());