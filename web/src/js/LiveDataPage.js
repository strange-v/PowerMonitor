import BasePage from './BasePage';
import Formatter from './Formatter';

export default class LiveDataPage extends BasePage {
    init() {
        this._initWebSocket();
    }

    _initWebSocket() {
        const protocol = window.location.protocol == 'https:' ? 'wss' : 'ws',
            gateway = `${protocol}://${window.location.hostname}:${window.location.port}/power/ws`;

        console.log('Trying to open a WebSocket connection...');
        this._ws = new WebSocket(gateway);
        this._ws.onopen = this._onOpen.bind(this);
        this._ws.onclose = this._onClose.bind(this);
        this._ws.onmessage = this._onMessage.bind(this);
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
