import scss from './Menu.scss';
import nativeEventsMixin from 'app/mixins/NativeEventsMixin';

export default class Menu {
    constructor(config) {
        Object.assign(this, nativeEventsMixin);

        this._cfg = config;

        this.addListener(this._cfg.el, 'click', this._onMenuButtonClick);

        this._menu = document.createElement('ul');
        this._menu.classList.add('menu');
        this._initMenuItems();
    }

    show() {
        if (!this._rendered) {
            this._rendered = true;
            this._cfg.el.appendChild(this._menu);
        }
        
        this._menu.classList.add('shown');
    }

    hide() {
        this._menu.classList.remove('shown');
    }

    _onMenuButtonClick(e) {
        this.show();

        if (this._destroyOutsideMenuClick) return;
        this._destroyOutsideMenuClick = this.subscribe(document.body, 'click', this._onOutsideMenuClick);
    }

    _onOutsideMenuClick(e) {
        if (!e.composedPath().includes(this._cfg.el)) {
            this.hide();
            this._destroyOutsideMenuClick();
            this._destroyOutsideMenuClick = null;
        }
    }

    _initMenuItems() {
        this._cfg.items.forEach(item => {
            const li = document.createElement('li');
            const a = document.createElement('a');
            a.innerHTML = item.iconCls
                ? `<i class="icon ${item.iconCls}"></i> ${item.text}`
                : `${item.text}`
            this.addListener(a, 'click', e => {
                this.hide();
                item.handler.apply(this._cfg.scope || this, [e]);
            });
            li.appendChild(a);
            this._menu.appendChild(li);
        });
    }
}