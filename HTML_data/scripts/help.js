// Unified Help Page - Works on ESP8266 and Server
const IS_SERVER = window.location.pathname.startsWith('/api/');
const API_PREFIX = IS_SERVER ? '/api' : '';

document.addEventListener("DOMContentLoaded", init);

let PinSetup = {};
let dataOther = {};
const MAX_COND_NUMBER = 4;

function getDeviceId() {
    const params = new URLSearchParams(window.location.search);
    return params.get('device_id') || window.DEVICE_ID || 'default';
}

function saveToLocalStorage() {
    localStorage.setItem('help_pinSetup', JSON.stringify(PinSetup));
    localStorage.setItem('help_dataOther', JSON.stringify(dataOther));
}

function init() {
    document.getElementById("btmBtns").appendChild(bottomButtons());
    loadPinSetup();
}

function loadPinSetup() {
    if (IS_SERVER) {
        const deviceId = getDeviceId();
        fetch(`/api/pin_setup?device_id=${deviceId}`)
            .then(res => res.json())
            .then(data => {
                PinSetup = data;
                saveToLocalStorage();
                loadOtherSetup();
            })
            .catch(() => {
                const stored = localStorage.getItem('help_pinSetup');
                if (stored) {
                    PinSetup = JSON.parse(stored);
                    loadOtherSetup();
                }
            });
    } else {
        readTextFile("pin_setup.txt", PinSetupLoaded);
    }
}

function PinSetupLoaded(data) {
    if (data == null) {
        setHTML("bodyNode", "Failed to load pin settings");
        return;
    }
    PinSetup = JSON.parse(data);
    saveToLocalStorage();
    loadOtherSetup();
}

function loadOtherSetup() {
    if (IS_SERVER) {
        const stored = localStorage.getItem('help_dataOther');
        if (stored) {
            dataOther = JSON.parse(stored);
        } else {
            dataOther = { deviceID: getDeviceId() };
        }
        loadBody();
        loadOtherHelp();
    } else {
        readTextFile("other_setup.txt", otherSetupLoaded);
    }
}

function loadOtherHelp() {
    const host = window.location.host;

    if (IS_SERVER) {
        setHTML("bodyNodeIR", "Not available on server");
        setHTML("bodyNodeStat", "Use /api/ajax endpoint");
        setHTML("bodyNodeEmail", "Not available on server");
        setHTML("bodyNodeSendIRw433", "Not available on server");
        setHTML("bodyNodeIRHex", "Not available on server");
        setHTML("bodyNodeIRRaw", "Not available on server");
        setHTML("bodyNodeActualTime", "Not available on server");
        setHTML("bodyNodeLed", "Not available on server");
        setHTML("bodyNodeReboot", "Not available on server");
        setHTML("wifi_mac_address", "N/A");
    } else {
        setHTML("bodyNodeIR", host + "/WaitIR");

        const link = document.createElement('a');
        link.href = "/sendAJAX?data={t:127,v:0}";
        link.textContent = host + "/sendAJAX?data={t:127,v:0}";
        const statNode = document.getElementById("bodyNodeStat");
        if (statNode) {
            statNode.innerHTML = "";
            statNode.appendChild(link);
        }

        setHTML("bodyNodeEmail", host + "/sendEmail?Email=Message text");
        setHTML("bodyNodeSendIRw433", host + "/sendAJAX?data={C:2,stat:-=Button number=-}");
        setHTML("bodyNodeIRHex", host + "/sendAJAX?data={C:3,st:-=HEX code=-}");
        setHTML("bodyNodeIRRaw", host + "/sendAJAX?data={C:4,st:-=Raw IR code=-}");
        setHTML("bodyNodeActualTime", host + "/setDate?DateTime={}");
        setHTML("bodyNodeLed", host + "/ws2811AJAX?data={\"from\":[0],\"to\":[88],\"type\":[2],\"dir\":[0],\"col\":[0],\"wh\":[254],\"br_\":[255],\"num\":1,\"sp\":100,\"dr\":255,\"fd\":45,\"fdt\":3,\"br\":12}");

        const rebootLink = document.createElement('a');
        rebootLink.href = "/function?data={reboot:1}";
        rebootLink.textContent = host + "/function?data={reboot:1}";
        const rebootNode = document.getElementById("bodyNodeReboot");
        if (rebootNode) {
            rebootNode.innerHTML = "";
            rebootNode.appendChild(rebootLink);
        }

        readTextFile("/function?data={'wifi_mac':1}", function (callback) {
            setHTML("wifi_mac_address", callback);
        });
    }
}

function otherSetupLoaded(data) {
    if (data == null) {
        setHTML("bodyNode", "Failed to load other settings");
        return;
    }
    dataOther = JSON.parse(data);
    saveToLocalStorage();
    loadBody();
    loadOtherHelp();
}


function createAdditionalLinks() {
    const container = document.createElement('div');
    const deviceId = "?device_id=" + getDeviceId();

    const links = IS_SERVER ? [
        { href: '#' + deviceId, text: 'OTA update', disabled: true },
        { href: '#' + deviceId, text: 'edit', disabled: true },
        { href: '/api/IR_setup' + deviceId, text: 'IR setup', disabled: false },
        { href: '#' + deviceId, text: 'Graphs', disabled: true },
        { href: '/api/homeassistant' + deviceId, text: 'homeassistant api', disabled: true }
    ] : [
        { href: '/update', text: 'OTA update' },
        { href: '/edit', text: 'edit' },
        { href: '/IR_setup', text: 'IR setup' },
        { href: '/graphs.htm', text: 'Graphs' },
        { href: '/homeassistant.htm', text: 'homeassistant api' }
    ];

    links.forEach(link => {
        const a = document.createElement('a');
        a.className = 'btn btn-block btn-default';
        a.href = link.href;
        a.textContent = link.text;
        if (link.disabled) {
            a.style.opacity = '0.5';
            a.style.cursor = 'not-allowed';
            a.onclick = (e) => { e.preventDefault(); alert('Not available on server'); };
        }
        container.appendChild(a);
    });

    return container;
}

function loadBody() {
    const table = document.createElement('table');
    table.className = 'table';
    table.style.width = '100%';

    const headerRow = document.createElement('tr');
    ['Description', 'Control by HTTP', 'Read HTTP'].forEach(text => {
        const td = document.createElement('td');
        td.textContent = text + ': ';
        headerRow.appendChild(td);
    });
    table.appendChild(headerRow);

    const host = window.location.host;
    const deviceId = dataOther.deviceID || getDeviceId();

    for (let i = 0; i < (PinSetup.numberChosed || 0); i++) {
        const row = document.createElement('tr');

        const descTd = document.createElement('td');
        descTd.textContent = PinSetup.descr[i] || '';
        row.appendChild(descTd);

        const controlTd = document.createElement('td');
        const code1 = document.createElement('code');
        const status_val = PinSetup.defaultVal[i] ^ 1;

        if (IS_SERVER) {
            const dataJson = JSON.stringify({ t: i, v: status_val });
            const http_request = `/api/ajax?data=${encodeURIComponent(dataJson)}&device_id=${deviceId}`;
            const link = document.createElement('a');
            link.href = http_request;

            const displayJson = JSON.stringify({ t: i, v: status_val });
            const displayUrl = `/api/ajax?data=${encodeURIComponent(displayJson)}&device_id=${deviceId}`;
            const parts = displayUrl.split(encodeURIComponent(String(status_val)));

            link.appendChild(document.createTextNode(host + parts[0]));
            const redSpan = document.createElement('span');
            redSpan.style.color = 'red';
            redSpan.textContent = '<value>';
            link.appendChild(redSpan);
            if (parts[1]) link.appendChild(document.createTextNode(parts[1]));

            code1.appendChild(link);
        } else {
            const http_request = `/sendAJAX?data={"t":${i},"v":${status_val}}`;
            const link = document.createElement('a');
            link.href = http_request;

            const parts = http_request.split(String(status_val));
            link.appendChild(document.createTextNode(host + parts[0]));
            const redSpan = document.createElement('span');
            redSpan.style.color = 'red';
            redSpan.textContent = '<value>';
            link.appendChild(redSpan);
            if (parts[1]) link.appendChild(document.createTextNode(parts[1]));

            code1.appendChild(link);
        }
        controlTd.appendChild(code1);
        row.appendChild(controlTd);

        const readTd = document.createElement('td');
        const code2 = document.createElement('code');
        if (IS_SERVER) {
            const dataJson = JSON.stringify({ t: i, v: 0 });
            const read_request = `/api/ajax?data=${encodeURIComponent(dataJson)}&device_id=${deviceId}`;
            const link = document.createElement('a');
            link.href = read_request;
            link.textContent = host + read_request;
            code2.appendChild(link);
        } else {
            const read_request = `/aRest?data={stat:${i}}`;
            const link = document.createElement('a');
            link.href = read_request;
            link.textContent = host + read_request;
            code2.appendChild(link);
        }
        readTd.appendChild(code2);
        row.appendChild(readTd);

        table.appendChild(row);
    }

    // Add "Read All" row
    const readAllRow = document.createElement('tr');
    const readAllDesc = document.createElement('td');
    readAllDesc.textContent = 'Read All Pins';
    readAllRow.appendChild(readAllDesc);

    const emptyTd = document.createElement('td');
    readAllRow.appendChild(emptyTd);

    const readAllTd = document.createElement('td');
    const code3 = document.createElement('code');
    if (IS_SERVER) {
        const dataJson = JSON.stringify({ t: 127, v: 0 });
        const read_all_request = `/api/ajax?data=${encodeURIComponent(dataJson)}&device_id=${deviceId}`;
        const link = document.createElement('a');
        link.href = read_all_request;
        link.textContent = host + read_all_request;
        code3.appendChild(link);
    } else {
        const read_all_request = `/sendAJAX?data={"t":127,"v":0}`;
        const link = document.createElement('a');
        link.href = read_all_request;
        link.textContent = host + read_all_request;
        code3.appendChild(link);
    }
    readAllTd.appendChild(code3);
    readAllRow.appendChild(readAllTd);
    table.appendChild(readAllRow);

    const bodyNode = document.getElementById('bodyNode');
    if (bodyNode) {
        bodyNode.innerHTML = '';
        bodyNode.appendChild(table);
    }

    const linksContainer = document.getElementById('additionalLinks');
    if (linksContainer) {
        linksContainer.innerHTML = '';
        linksContainer.appendChild(createAdditionalLinks());
    }
}

function readTextFile(file, callback) {
    const xmlHttp = new XMLHttpRequest();
    xmlHttp.overrideMimeType("application/json");
    xmlHttp.open("GET", file, true);
    xmlHttp.onreadystatechange = function () {
        if (xmlHttp.readyState === 4) {
            if (xmlHttp.status === 200) {
                callback(xmlHttp.responseText);
            } else {
                callback(null);
            }
        }
    }
    xmlHttp.send(null);
}

function setHTML(ID, value) {
    const elem = document.getElementById(ID);
    if (elem) {
        elem.innerHTML = value;
    }
}