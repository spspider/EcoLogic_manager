/**
 * Created by sergey on 24.09.2017.
 */
var Other_setup = {};
var Conditions = [{}];
var Pin_Setup = {};
var reloadPeriod = 10000;
var MAX_COND_NUMBER = 4;
var running = false
var defineFuel = 0;
DEBUG = false


/**
 * Инициализация страницы - загружает конфигурацию с сервера и создает кнопки управления
 */
async function firstload() {
    try {
        // Создаем кнопки управления периодом обновления
        makeStartStopButton();
        setHTML("btmBtns", bottomButtons());

        // Получаем device_id из URL или глобальной переменной
        const deviceId = window.DEVICE_ID || new URLSearchParams(window.location.search).get('device_id') || 'default';
        
        // Загружаем конфигурацию конкретного устройства
        try {
            const response = await fetch(`/api/config?id=${deviceId}`);
            if (response.ok) {
                const config = await response.json();
                if (config && config.numberChosed) {
                    createButtons_pin_setup(config);
                    document.title = `EcoLogic Remote - ${deviceId}`;
                } else {
                    console.error(`Конфигурация устройства ${deviceId} не найдена`);
                }
            } else {
                console.error('Сервер недоступен');
            }
        } catch (error) {
            console.error('Ошибка загрузки конфигурации:', error);
        }
        
        // Сохраняем device_id для использования в других функциях
        window.currentDeviceId = deviceId;
        
        // Запускаем периодический опрос состояния
        run();
    } catch (error) {
        console.error("Ошибка в firstload:", error);
    }
}


// Удалены неиспользуемые функции loadFileAsync и loadLicense




var LicenseCodeTimeout;
function setTimeoutLicenseCode() {
    StartProgram();
}

function StartProgram() {
    // sendAJAX(this, JSON.stringify({ t: 127, v: 0 }));
    run();
}


var myChart = [{}];
var DataChart = [{}];

var Graph = [{}];
var timeOut;
var timeOut_answer;

var running = false;

function clearMyTimeout() {
    clearTimeout(timeOut); //останавливаем слудующий таймер, так как этот таймер не остановлен
    running = false;
    clearTimeout(timeOut_answer);
}

function createButtons_pin_setup(data) {
    Pin_Setup = data;
    var n = data.numberChosed;
    //alert(n);
    var parsetext = {};

    for (i = 0; i < n; i++) {
        parsetext.id = i;
        parsetext.descr = data.descr[i];
        parsetext.widget = data.widget[i];
        parsetext.topic = parsetext.id + "/" + parsetext.widget + "/" + parsetext.id;
        new ParseAndCreateButtons(parsetext);
    }
}

function loadfile(file, callbackPARSE) {
    var data = {};
    try {
        readTextFile(file, function (text) {
            try {
                data = JSON.parse(text);
                callbackPARSE(data);
                return data;
            } catch (e) {
                return null;
            }
        });
    } catch (e) {
        return null;
    }
}

function createXmlHttpObject() {
    if (window.XMLHttpRequest) {
        xmlHttp = new XMLHttpRequest();
    } else {
        xmlHttp = new ActiveXObject('Microsoft.XMLHTTP');
    }
    return xmlHttp;
}

function setReloadPeriod(thisItem) {
    reloadPeriod = thisItem.value;
    var refreshInput = document.getElementById("refresh-rate");
    set_cookie("reloadPeriod", reloadPeriod);
    refreshInput.value = reloadPeriod;
    refreshInput.onchange = function (e) {
        var value = parseInt(e.target.value);
        reloadPeriod = (value > 0) ? value : 0;
        e.target.value = reloadPeriod;
    }
    running = false;
    run();
}

function loadValuesRun_AJAX() {
    if (!running) return;
    
    var sendJSON = JSON.stringify({
        't': 127,
        'v': 0
    });
    
    if (running) {
        sendAJAX(false, sendJSON);
        
        clearTimeout(timeOut);
        timeOut = setTimeout(loadValuesRun_AJAX, reloadPeriod);
        clearTimeout(timeOut_answer);
        timeOut_answer = setTimeout(clearMyTimeout, (reloadPeriod + 10000));
    }
}

function run() {
    if (!running) {
        running = true;
        loadValuesRun_AJAX();
    }
}

function makeStartStopButton() {
    var refreshInput = document.getElementById("refresh-rate");
    reloadPeriod = parseInt(get_cookie("reloadPeriod"));
    reloadPeriod = (reloadPeriod < 1000 || isNaN(reloadPeriod)) ? 1000 : reloadPeriod;
    //reloadPeriod = isNaN(reloadPeriod)?1000:reloadPeriod;
    refreshInput.value = reloadPeriod;
    setVal("run_range", reloadPeriod);
    refreshInput.onchange = function (e) {
        var value = parseInt(e.target.value);
        reloadPeriod = (value > 0) ? value : 500;
        e.target.value = reloadPeriod;
    }
} Pin_Setup


function SetClassName(status, default_val) {
    var newstatus = status ^ default_val;
    var NewclassName = "btn btn-block btn-default";
    if (newstatus === 1) {
        NewclassName = parseInt(default_val) === 0 ? "btn btn-block btn-success" : "btn btn-block btn btn-primary";
    } else {
        NewclassName = "btn btn-block btn-default";
    }
    return NewclassName;
}

function setValueName(newstatus) {

    var name = "выкл";
    if (newstatus == 1) {
        name = "вкл";
    } else {
        name = "выкл";
    }
    return name;
}

var parsetext = {};

function SetNewStatus(Statusmessage) {
    parsetext = Statusmessage;
    var id = parsetext.id;
    if (!sTopic[id]) {
        return;
    }

    var nameWidget = Pin_Setup.widget[id];
    var NewStatus = 0;
    if (document.getElementById(parsetext.sTopic)) {
        NewStatus = document.getElementById(parsetext.sTopic);
    } else if (document.getElementById(nameWidget)) {
        NewStatus = document.getElementById(nameWidget);
    } else {
        return;
    }

    switch (Pin_Setup.widget[id]) {
        case "toggle":
        case 1:
            //NewStatus.value = Pin_Setup.descr[id] + " " + setValueName(parsetext.status ^ Pin_Setup.defaultVal[id]);
            NewStatus.value = Pin_Setup.descr[id] + " " + parsetext.status;
            NewStatus.className = SetClassName(parsetext.status, Pin_Setup.defaultVal[id]);
            break;
        case "range":
        case 3:
            NewStatus.value = parsetext.status;
            textID = sTopic[id] + "/text";
            setHTML(textID, parsetext.status);
            break;
        case "progress-bar":
        case 4:
            var progress = (parsetext.status * 100 / 1024);
            NewStatus.style = "width:" + progress + "%";
            textID = sTopic[id] + "/text";
            setHTML(textID, parsetext.status);
            break;
        case "simple-btn":
        case 2:
            NewStatus.value = sDescr[id] + " " + setValueName(parsetext.status ^ Pin_Setup.defaultVal[id]);
            NewStatus.className = SetClassName(parsetext.status, Pin_Setup.defaultVal[id]);
            break;
        case "chart":
        case 5:
            if (!isEmpty(Graph[id])) {
                Graph[id].add(parsetext.status);
            }
            if (!isEmpty(myChart[id])) {
                var date = new Date();
                var time = date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds();

                var random = Math.floor(Math.random() * (1000 - 10)) + 10;
                DataChart[id].datasets[0].data.push(parsetext.status);
                DataChart[id].labels.push(time);
                if (DataChart[id].datasets[0].data.length > 1000) {
                    DataChart[id].datasets[0].data.splice(0, 1);
                    DataChart[id].labels.splice(0, 1);
                }
                DataChart[id].datasets[0].label = Pin_Setup.descr[id] + ": " + parsetext.status;
                myChart[id].update();
            }
            break;

        case 6://данные - текст

            NewStatus.innerHTML = Pin_Setup.descr[id] + ":" + "<h1>" + parsetext.status + "</h1>";
            break;
        case deault:
            NewStatus.innerHTML = Pin_Setup.descr[id] + ":" + "<h1>" + parsetext.status + "</h1>";
            //NewStatus.value = sDescr[id] + " " + "unknown";
            break;
    }
    sStatus(id, parsetext.status);
}

function isEmpty(obj) {
    for (var prop in obj) {
        if (obj.hasOwnProperty(prop))
            return false;
    }

    return true;
}

function ParseAndCreateButtons(parsetext) {
    setHTML("demo", getHTML("demo") + "</br>");
    sTopic(parsetext.id, parsetext.topic);
    sDescr(parsetext.id, parsetext.descr);
    sWidget(parsetext.id, parsetext.widget);
    sId(parsetext.topic, parsetext.id);
    var id = parsetext.id;
    var makeWidjet = "";
    textID = parsetext.topic + "/text";
    switch (parsetext.widget) {

        case 'toggle':
        case 1:
            makeWidjet = "<input id='" + parsetext.topic + "' class='btn btn-block btn-success' type='button' value='" + parsetext['descr'] + "' onclick='sendNewValue(this," + parsetext.id + ");' /></br>";
            break;
        case 'range':
        case 3:
            makeWidjet = parsetext.descr + " <p1 id='" + textID + "'></p1><input id='" + parsetext.topic + "' class='form-control'  type='range' min='0' max='1024' step='1' onchange='sendNewValue(this," + parsetext.id + ");'/></br>";
            break;
        case 'progress-bar':
        case 4:
            var value_max = 1024;
            var value_min = 0;
            if (Pin_Setup.pin[id] === 17) {
                value_max = 1024 / (Pin_Setup.aDiv ? parseInt(Pin_Setup.aDiv) : 1) - (!isNaN(parseInt(Pin_Setup.aSusbt)) ? parseInt(Pin_Setup.aSusbt) : 0);
                value_min = (!isNaN(parseInt(Pin_Setup.aSusbt)) ? parseInt(Pin_Setup.aSusbt) : 0);
            }
            //var value_max=1;
            makeWidjet = parsetext.descr + "<div class='progress'><div id='" + parsetext.topic + "' class='progress-bar' role='progressbar' aria-valuenow='0' aria-valuemin='" + value_min + "' aria-valuemax='" + value_max + "' style='width:100%'><p1 id='" + textID + "'></p1></div></div>";
            break;
        case 'simple-btn':
        case 2:
            makeWidjet = "<input id='" + parsetext.topic + "' class='btn btn-block btn-success' type='button' value='" + parsetext['descr'] + "' onmousedown='mouseDownBtn(" + parsetext.id + ",this)' onmouseup='mouseUpBtn(" + parsetext.id + ",this)' /></br>";
            break;
        case 'chart':
        case 5:
            if (!isEmpty(Chart)) {
                var that_topic = parsetext.topic;
                var nameChart = that_topic;
                var countup = this;
                var newNode = document.createElement('canvas');
                newNode.className = nameChart;
                newNode.id = nameChart;
                document.getElementById("charts").appendChild(newNode);
                var ctx = document.getElementById(nameChart).getContext('2d');
                DataChart[parsetext.id] = {
                    labels: [],
                    datasets: [{
                        data: [],
                        fill: false,
                        label: parsetext.descr,
                        radius: 0,
                        backgroundColor: "rgba(33, 170, 191,1)",
                        borderColor: "rgba(33, 170, 191,1)"
                    }]
                };
                try {
                    myChart[parsetext.id] = new Chart(ctx, {
                        type: 'line',
                        data: DataChart[parsetext.id],
                        options: {
                            tooltips: {
                                mode: 'index',
                                intersect: false
                            }
                        }
                    });
                } catch (e) {
                }
            } else {

            }
            break;
        case 6://ADC
            makeWidjet = "<div id='" + parsetext.topic + "'/></div></br>";
            break;
        case 7:
            if (typeof createGraph !== "undefined") {
                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!EEEEEEEEEEEEEERRRRRRRRRRRRRRRRRROOOOOOOOOOOORRRRRRRRRR
                var g = document.createElement('div');
                g.v = 0;
                g.setAttribute("id", "" + parsetext.topic);
                var mainGraph = document.getElementById("graph");
                mainGraph.appendChild(g);
                Graph[parsetext.id] = createGraph(document.getElementById("" + parsetext.topic), "Analog Input", 100, 128, 0, 1023, false, "cyan");
            }
            break;
        default:
    }

    if (makeWidjet) {
        setHTML("demo", getHTML("demo") + makeWidjet);//" pin: "+Pin_Setup.pin[id]+" "+
    }

}

var Cond_load = 0;


var timeOut_saveCond;

var timeOut_button = [];

function mouseDownBtn(id, button) {
    //sStatus[id] = switchToggle(Pin_Setup.defaultVal[id] ^ 1);
    sStatus[id] = switchToggle(Pin_Setup.defaultVal[id] ^ 1);
    button.className = ('btn btn-block btn btn-danger');
    sendNewValue(button, id);
    timeOut_button[id] = setTimeout(mouseUpBtn, 10000, id, button);
}

function mouseUpBtn(id, button) {
    clearTimeout(timeOut_button[id]);
    sStatus[id] = switchToggle(Pin_Setup.defaultVal[id] ^ 0);
    //sStatus[id] = (1);
    if (Pin_Setup.defaultVal[id] ^ 1 === 1) {
        button.className = ('btn btn-block btn btn-success');
    }
    if (Pin_Setup.defaultVal[id] ^ 1 === 0) {
        button.className = ('btn btn-block btn btn-primary');
    }
    sendNewValue(button, id);
}

//var sTopic=[];
function sTopic(id, topic) {
    sTopic[id] = topic;
}

function sWidget(id, widget) {
    sWidget[id] = widget;
}

function sDescr(id, descr) {
    sDescr[id] = descr;
}

function sId(topic, id) {
    sId[topic] = id;
}

function sStatus(id, status) {
    sStatus[id] = status;
}

function switchToggle(value) {
    if (value >= 1) {
        value = 1;
    } else if (value <= 0) {
        value = 0;
    }
    return value ^ 1;
}

function sendNewValue(button, id) {
    //PreVvalue = parseInt(sStatus[id]);
    //PreVvalue = parseInt(Pin_Setup.defaultVal[id]);
    PreVvalue = sStatus[id]; //& Pin_Setup.defaultVal[id];
    //PreVvalue =0;
    var topic = id + "/" + Pin_Setup.widget[id] + "/" + id;
    NewValue = getVal(topic);
    textID = topic + "/t";
    setHTML(textID, NewValue);
    switch (sWidget[id]) {
        case "toggle":
        case 1:
            setvalue = switchToggle(PreVvalue);
            break;
        case "simple-btn":
        case 2:
            setvalue = switchToggle(PreVvalue);
            break;
        case "range":
        case 3:
            setvalue = NewValue;
            break;
        case "chart":
        case 5:
            break;
    }
    sendJSON = JSON.stringify({
        't': id,
        'v': setvalue
    });

    setHTML("input", sendJSON);

    sendAJAX(button = button, sendJSON = sendJSON);

}

function sendAJAX(submit, sendJSON) {
    const deviceId = window.currentDeviceId || 'default';
    server = `/api/ajax?data=${encodeURIComponent(sendJSON)}&device_id=${deviceId}`;

    readTextFile(server, (responseText) => RespondCode(responseText, server, sendJSON));
    return false;
}

function RespondCode(responseText, server, sendJSON) {
    if (responseText === null) {
        clearMyTimeout();
    }
    setHTML("output", responseText);
    try {
        const parsedResponse = JSON.parse(responseText);
        const newStatusText = {};

        for (let i = 0; i < parsedResponse.stat.length; i++) {
            newStatusText.sTopic = `${i}/${Pin_Setup.widget[i]}/${i}`;
            newStatusText.status = parseFloat(parsedResponse.stat[i].toString());
            newStatusText.id = i;
            newStatusText.widget = Pin_Setup.widget[i];
            SetNewStatus(newStatusText);
        }
    } catch (e) {// in case of text plain value coming
        try {
            const newStatusText = {};
            const request = JSON.parse(sendJSON);
            newStatusText.id = request.t;
            newStatusText.status = responseText
            newStatusText.sTopic = `${request.t}/${Pin_Setup.widget[request.t]}/${request.t}`;
            newStatusText.widget = Pin_Setup.widget[request.t];
            SetNewStatus(newStatusText);
        }
        catch (e) {
            setHTML(
                "output",
                getHTML("output") +
                `<div> Server: ${server}<br>Request: ${sendJSON}</div>`
            );
        }
    }
}

