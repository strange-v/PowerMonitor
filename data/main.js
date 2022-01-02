const protocol = window.location.protocol == 'https:' ? 'wss' : 'ws';
const gateway = `${protocol}://${window.location.hostname}:${window.location.port}${window.location.pathname}ws`;
let websocket;

if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register(`${window.location.pathname}service-worker.js`);
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}
function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    const data = JSON.parse(event.data),
        power = document.getElementById('power'),
        current = document.getElementById('current'),
        voltage = document.getElementById('voltage'),
        frequency = document.getElementById('frequency'),
        pf = document.getElementById('pf'),
        energy = document.getElementById('energy'),
        uptime = document.getElementById('uptime');

    power.innerText = `${data.p} W`;
    current.innerText = `${data.c} A`;
    voltage.innerText = `${data.v} V`;
    frequency.innerText = `${data.f} Hz`;
    pf.innerText = `${data.pf}`;
    energy.innerText = `${data.e} kWh`;
    uptime.innerText = formatUptime(data.u);

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
        
}

function onLoad(event) {
    initWebSocket();
}

function formatUptime(value) {
    const days = Math.floor(value / (24 * 60 * 60));
    const hours = Math.floor(value / (60 * 60));
    const minutes = Math.floor(value / 60) % 60;
    const seconds = value % 60;

    const result = days > 1
        ? [{ value: days, suffix: 'd' },
        { value: hours, suffix: 'h' },
        { value: minutes, suffix: 'm' }]
        : [{ value: hours, suffix: 'h' },
        { value: minutes, suffix: 'm' },
        { value: seconds, suffix: 's' }]

    return result
        .map(data => {
            return `${data.value}${data.suffix}`
        })
        .join(' ')
}

window.addEventListener('load', onLoad);