// Universal Home Script - Works on ESP8266 and Server
// Auto-detects environment by checking URL
const IS_SERVER = window.location.pathname.startsWith('/api/');
const API_PREFIX = IS_SERVER ? '/api' : '';

// Get device_id from URL if on server
function getDeviceId() {
    const params = new URLSearchParams(window.location.search);
    return params.get('device_id') || window.DEVICE_ID || 'default';
}

let pinSetup = {};
let reloadPeriod = 1000;
let running = false;
let timeOut, timeOut_answer;
const DEBUG = false;

// Widget state storage
const widgetState = {
    topics: [],
    widgets: [],
    descriptions: [],
    statuses: [],
    charts: {},
    chartData: {},
    graphs: {}
};

document.addEventListener('DOMContentLoaded', init);

function init() {
    makeStartStopButton();
    document.getElementById("btmBtns").appendChild(bottomButtons());

    const deviceId = getDeviceId();
    const filePath = IS_SERVER ? `/api/pin_setup?device_id=${deviceId}` : "pin_setup.txt";

    if (IS_SERVER) {
        fetch(filePath)
            .then(res => res.json())
            .then(data => {
                pinSetup = data;
                createButtons();
                sendAJAX(null, JSON.stringify({ t: 127, v: 0 }));
                run();
            })
            .catch(error => console.error("Init error:", error));
    } else {
        readTextFile(filePath, (text) => {
            if (text) {
                try {
                    pinSetup = JSON.parse(text);
                    createButtons();

                    readTextFile("function?data={\"get_device_id\":1}", (deviceId) => {
                        localStorage.setItem('deviceID', deviceId);
                        document.title = deviceId;
                    });
                } catch (error) {
                    console.error("Init error:", error);
                }
            }
            sendAJAX(null, JSON.stringify({ t: 127, v: 0 }));
            run();
        });
    }
}

function createButtons() {
    const n = pinSetup.numberChosed || 0;

    for (let i = 0; i < n; i++) {
        const widget = {
            id: i,
            descr: pinSetup.descr[i],
            widget: pinSetup.widget[i],
            topic: `${i}/${pinSetup.widget[i]}/${i}`
        };
        createWidget(widget);
    }
}

function createWidget(widget) {
    const { id, descr, widget: widgetType, topic } = widget;

    widgetState.topics[id] = topic;
    widgetState.descriptions[id] = descr;
    widgetState.widgets[id] = widgetType;

    const container = document.getElementById("demo");
    const textID = `${topic}/text`;
    let element;

    switch (widgetType) {
        case 'toggle':
        case 1:
            element = document.createElement('input');
            element.id = topic;
            element.className = 'btn btn-block btn-success';
            element.type = 'button';
            element.value = descr;
            element.onclick = () => sendNewValue(element, id);
            break;

        case 'range':
        case 3:
            const rangeContainer = document.createElement('div');
            const label = document.createElement('span');
            label.textContent = descr + ' ';
            const valueDisplay = document.createElement('span');
            valueDisplay.id = textID;
            const range = document.createElement('input');
            range.id = topic;
            range.className = 'form-control';
            range.type = 'range';
            range.min = '0';
            range.max = '1024';
            range.step = '1';
            range.onchange = () => sendNewValue(range, id);
            rangeContainer.appendChild(label);
            rangeContainer.appendChild(valueDisplay);
            rangeContainer.appendChild(range);
            element = rangeContainer;
            break;

        case 'progress-bar':
        case 4:
            let valueMax = 1024;
            let valueMin = 0;
            if (pinSetup.pin[id] === 17) {
                valueMax = 1024 / (pinSetup.aDiv || 1) - (pinSetup.aSusbt || 0);
                valueMin = pinSetup.aSusbt || 0;
            }
            const progressContainer = document.createElement('div');
            progressContainer.innerHTML = descr;
            const progressDiv = document.createElement('div');
            progressDiv.className = 'progress';
            const progressBar = document.createElement('div');
            progressBar.id = topic;
            progressBar.className = 'progress-bar';
            progressBar.setAttribute('role', 'progressbar');
            progressBar.setAttribute('aria-valuenow', '0');
            progressBar.setAttribute('aria-valuemin', valueMin);
            progressBar.setAttribute('aria-valuemax', valueMax);
            progressBar.style.width = '100%';
            const progressText = document.createElement('span');
            progressText.id = textID;
            progressBar.appendChild(progressText);
            progressDiv.appendChild(progressBar);
            progressContainer.appendChild(progressDiv);
            element = progressContainer;
            break;

        case 'simple-btn':
        case 2:
            element = document.createElement('input');
            element.id = topic;
            element.className = 'btn btn-block btn-success';
            element.type = 'button';
            element.value = descr;
            element.onmousedown = () => mouseDownBtn(id, element);
            element.onmouseup = () => mouseUpBtn(id, element);
            break;

        case 'chart':
        case 5:
            if (typeof Chart !== 'undefined') {
                const canvas = document.createElement('canvas');
                canvas.id = topic;
                canvas.className = topic;
                document.getElementById("charts").appendChild(canvas);

                widgetState.chartData[id] = {
                    labels: [],
                    datasets: [{
                        data: [],
                        fill: false,
                        label: descr,
                        radius: 0,
                        backgroundColor: "rgba(33, 170, 191,1)",
                        borderColor: "rgba(33, 170, 191,1)"
                    }]
                };

                try {
                    const ctx = canvas.getContext('2d');
                    widgetState.charts[id] = new Chart(ctx, {
                        type: 'line',
                        data: widgetState.chartData[id],
                        options: {
                            tooltips: {
                                mode: 'index',
                                intersect: false
                            }
                        }
                    });
                } catch (e) {
                    console.error("Chart creation error:", e);
                }
            }
            return;

        case 6:
            element = document.createElement('div');
            element.id = topic;
            break;

        case 7:
            if (typeof createGraph !== 'undefined') {
                const graphDiv = document.createElement('div');
                graphDiv.id = topic;
                document.getElementById("graph").appendChild(graphDiv);
                widgetState.graphs[id] = createGraph(graphDiv, "Analog Input", 100, 128, 0, 1023, false, "cyan");
            }
            return;

        default:
            return;
    }

    if (element) {
        container.appendChild(element);
        container.appendChild(document.createElement('br'));
    }
}

function setNewStatus(statusMsg) {
    const { id, status, sTopic, widget } = statusMsg;

    if (!widgetState.topics[id]) return;

    const element = document.getElementById(sTopic || widgetState.topics[id]);
    if (!element) return;

    widgetState.statuses[id] = status;
    saveToLocalStorage();

    switch (widget || widgetState.widgets[id]) {
        case 'toggle':
        case 1:
            element.value = `${pinSetup.descr[id]} ${status}`;
            element.className = getClassName(status, pinSetup.defaultVal[id]);
            break;

        case 'range':
        case 3:
            element.value = status;
            setHTML(`${widgetState.topics[id]}/text`, status);
            break;

        case 'progress-bar':
        case 4:
            const progress = (status * 100 / 1024);
            element.style.width = `${progress}%`;
            setHTML(`${widgetState.topics[id]}/text`, status);
            break;

        case 'simple-btn':
        case 2:
            element.value = `${pinSetup.descr[id]} ${status}`;
            element.className = getClassName(status, pinSetup.defaultVal[id]);
            break;

        case 'chart':
        case 5:
            if (widgetState.graphs[id]) {
                widgetState.graphs[id].add(status);
            }
            if (widgetState.charts[id]) {
                const date = new Date();
                const time = `${date.getHours()}:${date.getMinutes()}:${date.getSeconds()}`;

                widgetState.chartData[id].datasets[0].data.push(status);
                widgetState.chartData[id].labels.push(time);

                if (widgetState.chartData[id].datasets[0].data.length > 1000) {
                    widgetState.chartData[id].datasets[0].data.shift();
                    widgetState.chartData[id].labels.shift();
                }

                widgetState.chartData[id].datasets[0].label = `${pinSetup.descr[id]}: ${status}`;
                widgetState.charts[id].update();
            }
            break;

        case 6:
            element.innerHTML = `${pinSetup.descr[id]}: <h1>${status}</h1>`;
            break;

        default:
            element.innerHTML = `${pinSetup.descr[id]}: <h1>${status}</h1>`;
            break;
    }
}

function getClassName(status, defaultVal) {
    const newStatus = status ^ defaultVal;
    if (newStatus === 1) {
        return parseInt(defaultVal) === 0 ? 'btn btn-block btn-success' : 'btn btn-block btn-primary';
    }
    return 'btn btn-block btn-default';
}

function sendNewValue(button, id) {
    const prevValue = widgetState.statuses[id];
    const topic = widgetState.topics[id];
    const newValue = getVal(topic);

    setHTML(`${topic}/text`, newValue);

    let setValue;
    switch (widgetState.widgets[id]) {
        case 'toggle':
        case 1:
        case 'simple-btn':
        case 2:
            setValue = switchToggle(prevValue);
            break;
        case 'range':
        case 3:
            setValue = newValue;
            break;
        default:
            return;
    }

    widgetState.statuses[id] = setValue;
    saveToLocalStorage();

    // Immediate visual feedback
    setNewStatus({
        sTopic: topic,
        status: setValue,
        id: id,
        widget: widgetState.widgets[id]
    });

    if (IS_SERVER) {
        const alertDiv = alert_message("Command sent, waiting for device sync...", 3);
        document.body.appendChild(alertDiv);
    }

    const sendJSON = JSON.stringify({ t: id, v: setValue });
    setHTML("input", sendJSON);
    sendAJAX(button, sendJSON);
}

function switchToggle(value) {
    return (value >= 1 ? 1 : 0) ^ 1;
}

let timeOutButton = [];

function mouseDownBtn(id, button) {
    widgetState.statuses[id] = switchToggle(pinSetup.defaultVal[id] ^ 1);
    button.className = 'btn btn-block btn-danger';
    sendNewValue(button, id);
    timeOutButton[id] = setTimeout(() => mouseUpBtn(id, button), 10000);
}

function mouseUpBtn(id, button) {
    clearTimeout(timeOutButton[id]);
    widgetState.statuses[id] = switchToggle(pinSetup.defaultVal[id] ^ 0);
    button.className = pinSetup.defaultVal[id] ^ 1 === 1 ? 'btn btn-block btn-success' : 'btn btn-block btn-primary';
    sendNewValue(button, id);
}

function sendAJAX(submit, sendJSON) {
    let server;
    if (IS_SERVER) {
        const deviceId = getDeviceId();
        server = `/api/ajax?data=${sendJSON}&device_id=${deviceId}`;
    } else {
        server = `/sendAJAX?data=${sendJSON}`;
    }

    if (DEBUG) {
        const statRespond = Array(pinSetup.numberChosed).fill("1.00");
        const respondCode = JSON.stringify({ stat: statRespond });
        console.log(respondCode);
        respondCode(respondCode, server, sendJSON);
    } else {
        readTextFile(server, (responseText) => respondCode(responseText, server, sendJSON));
    }
    return false;
}

function respondCode(responseText, server, sendJSON) {
    if (responseText === null) {
        clearMyTimeout();
        return;
    }

    setHTML("output", responseText);

    try {
        const parsedResponse = JSON.parse(responseText);

        if (IS_SERVER && parsedResponse.has_updates == 1) {
            console.log("Pending changes, waiting for device sync...");
            return;
        }

        if (parsedResponse.stat && Array.isArray(parsedResponse.stat)) {
            for (let i = 0; i < parsedResponse.stat.length; i++) {
                setNewStatus({
                    sTopic: widgetState.topics[i],
                    status: parseFloat(parsedResponse.stat[i]),
                    id: i,
                    widget: pinSetup.widget[i]
                });
            }
        }
    } catch (e) {
        const request = JSON.parse(sendJSON);
        const statusValue = parseFloat(responseText);
        if (!isNaN(statusValue)) {
            setNewStatus({
                id: request.t,
                status: statusValue,
                sTopic: widgetState.topics[request.t],
                widget: pinSetup.widget[request.t]
            });
        }
    }
}

function run() {
    if (!running) {
        running = true;
        loadValuesRun();
    }
}

function loadValuesRun() {
    if (!running) return;

    const sendJSON = JSON.stringify({ t: 127, v: 0 });
    sendAJAX(false, sendJSON);

    clearTimeout(timeOut);
    timeOut = setTimeout(loadValuesRun, reloadPeriod);

    clearTimeout(timeOut_answer);
    timeOut_answer = setTimeout(clearMyTimeout, reloadPeriod + 10000);
}

function clearMyTimeout() {
    clearTimeout(timeOut);
    clearTimeout(timeOut_answer);
    running = false;
}

function setReloadPeriod(element) {
    const value = parseInt(element.value);
    reloadPeriod = value > 0 ? value : 500;
    document.getElementById("refresh-rate").value = reloadPeriod;
    set_cookie("reloadPeriod", reloadPeriod);
    running = false;
    run();
}

function makeStartStopButton() {
    const refreshInput = document.getElementById("refresh-rate");
    const savedPeriod = parseInt(get_cookie("reloadPeriod"));
    reloadPeriod = (savedPeriod >= 1000 && !isNaN(savedPeriod)) ? savedPeriod : 1000;

    refreshInput.value = reloadPeriod;
    setVal("run_range", reloadPeriod);

    refreshInput.onchange = (e) => {
        const value = parseInt(e.target.value);
        reloadPeriod = value > 0 ? value : 500;
        e.target.value = reloadPeriod;
        set_cookie("reloadPeriod", reloadPeriod);
        running = false;
        run();
    };
}

function saveToLocalStorage() {
    const stateToSave = {
        topics: widgetState.topics,
        widgets: widgetState.widgets,
        descriptions: widgetState.descriptions,
        statuses: widgetState.statuses
    };
    localStorage.setItem('widgetState', JSON.stringify(stateToSave));
}
