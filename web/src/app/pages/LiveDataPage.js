import BasePage from './BasePage';
import Formatter from '../utils/Formatter';
import WebSocketWrapper from '../utils/WebSocketWrapper';
import Confirmation from '../components/confirmation/Confirmation';
import Menu from '../components/menu/Menu';
import html from './LiveDataPage.html';
import scss from './LiveDataPage.scss';

export default class LiveDataPage extends BasePage {
    init() {
        super.init(html);
        this._initWebSocket();

        this._menu = new Menu({
            el: document.getElementById('btn-menu'),
            items: [{
                text: 'Reset Energy',
                iconCls: 'reset',
                handler: this._onResetEnergyClick
            }, {
                text: 'Settings',
                iconCls: 'settings',
                handler: this._onSettingsClick
            }],
            scope: this
        });
    }

    destroy() {
        this._ws.destroy();
        super.destroy();
    }

    _onResetEnergyClick(e) {
        e.stopPropagation();

        const confirmation = new Confirmation({
            text: 'Are you sure you want to reset the energy counter?',
            listeners: {
                confirm: this._resetEnergy,
                scope: this
            }
        });
        confirmation.show(document.getElementById('app'));
    }

    _onSettingsClick(e) {
        e.stopPropagation();

        this.redirectTo('/settings');
    }

    _resetEnergy() {
        this._ws.suppressEvents(true);
        this.showLoading();

        fetch('/power/api/resetEnergy', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: ''
        }).then(() => {
            this._ws.suppressEvents(false);
        });
    }

    _initWebSocket() {
        const protocol = window.location.protocol == 'https:' ? 'wss' : 'ws',
            gateway = `${protocol}://${window.location.hostname}:${window.location.port}/power/ws`;

        this._ws = new WebSocketWrapper(gateway, { reconnect: 2000 });
        this._ws.on('message', this._onMessage, this);
    }

    _onMessage(e) {
        const data = JSON.parse(e.data),
            power = document.getElementById('power'),
            current = document.getElementById('current'),
            voltage = document.getElementById('voltage'),
            frequency = document.getElementById('frequency'),
            pf = document.getElementById('pf'),
            energy = document.getElementById('energy'),
            uptime = document.getElementById('uptime');

        power.innerText = Formatter.power(data.p);
        current.innerText = Formatter.current(data.c);
        voltage.innerText = `${data.v} V`;
        frequency.innerText = `${data.f} Hz`;
        pf.innerText = `${data.pf}`;
        energy.innerText = Formatter.energy(data.e);
        uptime.innerText = Formatter.uptime(data.u);

        if (data.pw)
            power.parentElement.classList.add('warning')
        else
            power.parentElement.classList.remove('warning');

        if (data.cw)
            current.parentElement.classList.add('warning')
        else
            current.parentElement.classList.remove('warning');

        if (data.vw)
            voltage.parentElement.classList.add('warning')
        else
            voltage.parentElement.classList.remove('warning');

        this.showContent();
    }
}
