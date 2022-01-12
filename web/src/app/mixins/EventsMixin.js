export default {
    on(eventName, handler, scope) {
        if (!this._eventHandlers) this._eventHandlers = {};
        if (!this._eventHandlers[eventName]) {
            this._eventHandlers[eventName] = [];
        }
        this._eventHandlers[eventName].push({ handler, scope });
    },

    un(eventName, handler, scope) {
        let handlers = this._eventHandlers?.[eventName];
        if (!handlers) return;
        for (let i = 0; i < handlers.length; i++) {
            if (handlers[i].handler === handler && handlers[i].scope === scope) {
                handlers.splice(i--, 1);
            }
        }
    },

    trigger(eventName, ...args) {
        if (!this._eventHandlers?.[eventName]) return;

        this._eventHandlers[eventName].forEach(cfg => cfg.handler.apply(cfg.scope || this, args));
    },

    destroy() {
        this._eventHandlers = null;
    }
}