export default {
    addListener(el, event, callback, scope) {
        if (this._listeners === undefined)
            this._listeners = [];
        
        if (typeof el === 'function')
            this._listeners.push(el);
        else
            this._listeners.push(this.subscribe(el, event, callback, scope));
    },

    subscribe(el, event, callback, scope) {
        scope = scope || this;

        if (typeof el === 'string')
            el = document.getElementById(el);

        const fn = callback.bind(this);
        el.addEventListener(event, fn, false);

        return () => el.removeEventListener(event, fn, false);
    },

    removeAllListeners() {
        if (this._listeners) {
            this._listeners.forEach(listener => listener());
        }
    }
}