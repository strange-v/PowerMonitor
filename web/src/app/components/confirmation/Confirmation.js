import html from './Confirmation.html';
import scss from './Confirmation.scss';
import eventsMixin from "app/mixins/EventsMixin";

export default class Confirmation {
    constructor(text, title, yes, no) {
        Object.assign(this, eventsMixin);

        if (typeof text == 'object') {
            const cfg = text;
            this.yes = cfg.yes || 'Yes';
            this.no = cfg.no || 'Cancel';
            this.title = cfg.title || 'Confirmation';
            this.text = cfg.text;
            this._attachListeners(cfg.listeners);
        } else {
            this.yes = yes || 'Yes';
            this.no = no || 'Cancel';
            this.title = title || 'Confirmation';
            this.text = text;
        }
    }

    show(el) {
        this._confirmation = document.createElement('div')
        this._confirmation.classList.add('app-dialog');
        this._confirmation.innerHTML = html
            .replace(['{Title}'], [this.title])
            .replace(['{Text}'], [this.text])
            .replace(['{Yes}'], [this.yes])
            .replace(['{No}'], [this.no])

        el.appendChild(this._confirmation);
        const dialog = this._confirmation.children[0];
        const marginTop = (document.body.clientHeight - dialog.offsetHeight) / 2;
        dialog.style.marginTop = marginTop - marginTop * 0.1;

        this._confirmation.getElementsByClassName('btn-no')[0].addEventListener('click', () => this._onAction(false), {
            once: true,
            capture: true
        });
        this._confirmation.getElementsByClassName('btn-yes')[0].addEventListener('click', () => this._onAction(true), {
            once: true,
            capture: true
        });
    }

    _attachListeners(listeners) {
        if (!listeners) return;

        const scope = listeners.scope || this;
        for (const [eventName, handler] of Object.entries(listeners)) {
            if (eventName === 'scope') continue;
            this.on(eventName, handler, scope);
        }
    }

    _onAction(response) {
        this._confirmation.remove();
        this.trigger('change', response);
        if (response) this.trigger('confirm');
        this.destroy();
    }

    destroy() {
        self.destroy();
    }
}
