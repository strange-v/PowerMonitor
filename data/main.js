'use strict'

const protocol = window.location.protocol == 'https:' ? 'wss' : 'ws';
const gateway = `${protocol}://${window.location.hostname}:${window.location.port}/power/ws`;
let websocket;

if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register(`/power/service-worker.js`);
}

class Formatter {
    static {
        this.uptime = (value) => {
            const days = Math.floor(value / (24 * 60 * 60));
            let hours = Math.floor(value / (60 * 60));
            const minutes = Math.floor(value / 60) % 60;
            const seconds = value % 60;
        
            let result = [{ value: hours, suffix: 'h' },
            { value: minutes, suffix: 'm' },
            { value: seconds, suffix: 's' }];
        
            if (days > 1) {
                hours = hours - days * 24;
                result = [{ value: days, suffix: 'd' },
                { value: hours, suffix: 'h' },
                { value: minutes, suffix: 'm' }];
            }
        
            return result
                .map(data => {
                    return `${data.value}${data.suffix}`
                })
                .join(' ')
        }
        this.power = (v) => this.autoNumber(v, 1, ['W', 'KW', 'MW'])
        this.current = (v) => this.autoNumber(v, 2, ['A', 'kA'])
        this.energy = (v) => this.autoNumber(v, 1, ['kWh', 'MWh'])
        this.autoNumber = (value, scale, units) => {
            let unitText = '';
            const currentUnit = Math.floor((Number(value).toFixed(0).length - 1) / 3) * 3;
            const unitName = units[Math.floor(currentUnit / 3)];
            const num = value / ('1e' + currentUnit);

            unitText = `${this.number(num, scale)} ${unitName}`;

            return unitText;
        }
        this.number = (v, scale, suffix = '') => {
            return v.toFixed(scale).replace(/([0-9]+(\.[0-9]+[1-9])?)(\.?0+$)/,'$1');
        }
    }
}

class BasePage {
    loading = true;

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
}

class LiveDataPage extends BasePage {
    init() {
        this._initWebSocket();
    }

    _initWebSocket() {
        console.log('Trying to open a WebSocket connection...');
        websocket = new WebSocket(gateway);
        websocket.onopen = this._onOpen.bind(this);
        websocket.onclose = this._onClose.bind(this);
        websocket.onmessage = this._onMessage.bind(this);
    }

    _onOpen(event) {
        console.log('Connection opened');
    }
    
    _onClose(event) {
        console.log('Connection closed');
        setTimeout(this._initWebSocket, 2000);
    }

    _onMessage(event) {
        const data = JSON.parse(event.data),
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

class SettingsPage extends BasePage {
    settings = {};

    init() {
        this._initControls();
        this._loadSettings();
    }

    _loadSettings() {
        fetch('/power/api/settings')
            .then(response => response.json())
            .then(data => this.settings = data)
            .then(_ => this._applySettings())
            .then(_ => this.showContent())
            .catch(e => console.log(e));
    }

    _onSave() {
        document.getElementById('form').requestSubmit();
    }

    _onSubmit(e) {
        e.preventDefault();

        this.showLoading();

        const data = this._getSettings();
        fetch('/power/api/settings', {
            method: 'PUT',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        }).then(() => this._loadSettings());
    }

    _getSettings() {
        const result = {};

        this._getFields().forEach(field => {
            const el = document.getElementById(field.id);

            if (field.deps) {
                const value = this._getFieldValue(el);
                const values = field.get(value);
                field.deps.forEach((prop, idx) => result[prop] = values[idx]);
                return;
            }

            result[field.prop] = this._getFieldValue(el);
        });

        return result;
    }

    _applySettings() {
        const fileds = this._getFields();
        
        fileds.forEach(field => {
            const el = document.getElementById(field.id);
            let value = this.settings[field.prop];

            if (field.deps) {
                const values = field.deps.map(prop => this.settings[prop]);
                value = values.some(v => v == '') ? '' : field.set(values);
            }

            this._setFieldValue(el, value);
        });
    }

    _initControls() {
        this._getFields().forEach(field => {
            if (field.refs) {
                const el = document.getElementById(field.id);
                el.onchange = () => field.refs.forEach(ref => this._disableFiled(ref, !this._getFieldValue(el)));
                field.refs.forEach(ref => this._disableFiled(ref, !this._getFieldValue(el)));
            }
        });
        document.getElementById('save').onclick = this._onSave.bind(this);
        document.getElementById('form').addEventListener('submit', (event) => this._onSubmit(event), false)
    }

    _disableFiled(id, disabled) {
        const el = document.getElementById(id);

        if (disabled) {
            el.disabled = disabled;
            el.parentElement.classList.add('disabled')
        } else {
            el.disabled = disabled;
            el.parentElement.classList.remove('disabled')
        }
    }

    _getFieldValue(field) {
        if (field.type == 'text' || field.type == 'password')
            return field.value;
        else if (field.type == 'number')
            return Number(field.value);
        else if (field.type == 'checkbox')
            return field.checked;
    }

    _setFieldValue(el, value) {
        if (el.type == 'text' || el.type == 'number' || el.type == 'password')
            el.value = value;
        else if (el.type == 'checkbox')
            el.checked = value;
        
        el.dispatchEvent(new Event('change'));
    }

    _getFields() {
        return [
            { id: 'voltageMin', prop: 'vMin' },
            { id: 'voltageMax', prop: 'vMax' },
            { id: 'powerMax', prop: 'pMax' },
            { id: 'currentMax', prop: 'cMax' },
            { id: 'enableMqtt', prop: 'mqtt', refs: ['mqttServer', 'mqttUser', 'mqttPassword', 'mqttTopic'] },
            {
                id: 'mqttServer',
                deps: ['mqttHost', 'mqttPort'],
                get: value => value.split(':'),
                set: values => values.join(':')
            },
            { id: 'mqttUser', prop: 'mqttUsr' },
            { id: 'mqttPassword', prop: 'mqttPwd' },
            { id: 'mqttTopic', prop: 'mqttTopic' },
            { id: 'requestDataInterval', prop: 'rdi' },
            { id: 'otaPassword', prop: 'otaPwd' },
        ];
    }
}


