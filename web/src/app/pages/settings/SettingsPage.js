import BasePage from "app/pages/BasePage";
import html from './SettingsPage.html';
import scss from './SettingsPage.scss';

export default class SettingsPage extends BasePage {
    settings = {};

    init() {
        super.init(html);
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

        document.getElementById('save').addEventListener('click', () => this._onSave(), false);
        document.getElementById('form').addEventListener('submit', (event) => this._onSubmit(event), false);
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

    _getFieldValue(el) {
        if (el.type == 'text' || el.type == 'password')
            return el.value;
        else if (el.type == 'number' || el.type == 'range')
            return Number(el.value);
        else if (el.type == 'checkbox')
            return el.checked;
    }

    _setFieldValue(el, value) {
        if (el.type == 'text' || el.type == 'number' || el.type == 'password' || el.type == 'range')
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
            { id: 'ntpServer', prop: 'ntp' },
            { id: 'requestDataInterval', prop: 'rdi' },
            { id: 'otaPassword', prop: 'otaPwd' },
        ];
    }
}
