import LiveDataPage from "app/pages/liveData/LiveDataPage";
import SettingsPage from "app/pages/settings/SettingsPage";
import Router from "app/Router";
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
        let route;
        for (const [key, view] of Object.entries(this.routes)) {
            route = this._buildRoute(key, view)(e.route);

            if (route.matched) {
                if (this._view) {
                    this._view.destroy();
                    delete this._view;
                }
        
                this._view = new route.view;
                this._view.init(route.params);
                return;
            }
        }
        
        if (this.routes[e.route] === undefined) {
            throw 'Implement 404';
        }
    }

    _buildRoute(route, view) {
        const types = {
            int: { regexp: '[0-9.]+', get: v => parseInt(v, 10) },
            string: { regexp: '[a-z.-]+', get: v => v },
        };
        const names = [];

        const part = route.replace(/{([a-z]+):([a-z]+)}/ig, (str, name, type) => {
            names.push({ name, get: types[type].get });
            return `(${types[type].regexp})`;
        });
        const regexp = new RegExp(`^${part}$`, 'i');

        return (hash) => {
            const match = hash.match(regexp);
            const result = {
                matched: match != null,
                view: view,
                params: {}
            }
            if (names) {
                result.hasParams = true;
                names.forEach((n, i) => {
                    result.params[n.name] = n.get(match[i + 1]);
                });
            }

            return result;
        };
    }
}

const app = new App;

document.addEventListener("DOMContentLoaded", () => app.init());