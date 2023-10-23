export default class Router extends EventTarget {
    constructor() {
        super();

        this._route = null;
        window.addEventListener('popstate', () => {
            if (this.getRoute() !== this._route) {
                this._route = this.getRoute();
                this._dispatchChangeEvent();
            }
        });
    }

    init() {
        this._route = this.getRoute();
        if (this._route == '')
            this.setRoute('/');
        else
            this._dispatchChangeEvent();
    }

    on(event, method, scope) {
        scope = scope === undefined ? this : scope;

        this.addEventListener(event, e => {
            method.bind(scope)(this, e.detail);
        });
    }

    setRoute(route) {
        window.location.hash = route;
        this._route = route;
    }

    getRoute() {
        return window.location.hash.substring(1);
    }

    _dispatchChangeEvent() {
        this.dispatchEvent(new CustomEvent('change', {
            bubbles: true,
            cancelable: false,
            detail: {
                route: this._route
            }
        }));
    }
}
