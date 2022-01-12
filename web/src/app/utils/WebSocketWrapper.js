import eventsMixin from "../mixins/EventsMixin";

export default class WebSocketWrapper {
    constructor(gateway, config) {
        Object.assign(this, eventsMixin);

        this._initConfig(gateway, config);

        this._onOpen = this._onWebSoketOpen.bind(this);
        this._onClose = this._onWebSoketClose.bind(this);
        this._onMessage = this._onWebSoketMessage.bind(this);
        this._initWebSocket();
    }

    destroy() {
        this.suppressEvents(true);
        clearTimeout(this._reconnect);
        this._ws.removeEventListener('open', this._onOpen, true);
        this._ws.removeEventListener('close', this._onClose, true);
        this._ws.removeEventListener('message', this._onMessage, true);
        this._ws.close();
        this._ws = null;
    }

    suppressEvents(suppress) {
        this._suppressEvents = suppress;
    }

    _onWebSoketOpen() {
        if (this._suppressEvents) return;
        this.trigger('open');
    }

    _onWebSoketClose() {
        if (!this._destroying && this._cfg.reconnect > 0) {
            this._reconnect = setTimeout(() => this._initWebSocket(), this._cfg.reconnect);
        }
        if (this._suppressEvents) return;
        this.trigger('close');
    }

    _onWebSoketMessage(data) {
        if (this._suppressEvents) return;
        this.trigger('message', data);
    }

    _initWebSocket() {
        if (this._ws) {
            this._ws.removeEventListener('open', this._onOpen, true);
            this._ws.removeEventListener('close', this._onClose, true);
            this._ws.removeEventListener('message', this._onMessage, true);
            this._ws.close();
            this._ws = null;
        }

        this._ws = new WebSocket(this._cfg.gateway);
        this._ws.addEventListener('open', this._onOpen, true);
        this._ws.addEventListener('close', this._onClose, true);
        this._ws.addEventListener('message', this._onMessage, true);
    }

    _initConfig(gateway, cfg) {
        this._cfg = {
            gateway,
            reconnect: 2000
        };

        Object.assign(this._cfg, cfg || {});
    }
}
