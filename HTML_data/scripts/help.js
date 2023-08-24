/**
 * Created by Башня1 on 17.02.2018.
 */
document.addEventListener("DOMContentLoaded", loadPinSetup);
var PinSetup = {};
var dataOther = {};
var Conditions = [{}];
var MAX_COND_NUMBER = 4;
var STAT = [];

function readSTAT() {
    //setHTML("btmBtns", bottomButtons());
    readTextFile("stat.txt", function (callback) {
        if (testJson(callback)) {
            STAT = JSON.parse(callback);
        }
    });

}

function loadPinSetup() {
    setHTML("btmBtns", bottomButtons());
    readTextFile("pin_setup.txt", PinSetupLoaded);

}

function PinSetupLoaded(data) {
    if (data == null) {
        setHTML("bodyNode", "не удалось загрузить настройки пинов");
        return;
    }
    readTextFile("other_setup.txt", other_setupLoaded);
    PinSetup = JSON.parse(data);
}

function loadOtherHelp() {
    var bodyNode = "";
    bodyNode = window.location.host + "/WaitIR";
    setHTML("bodyNodeIR", bodyNode);
    //var bodyNode="";
    bodyNode = "<a  href=/sendAJAX?json={t:127,v:0}>" + window.location.host + "/sendAJAX?json={t:127,v:0}</a>";
    setHTML("bodyNodeStat", bodyNode);
    //var bodyNode="";
    bodyNode = window.location.host + "/sendEmail?Email=текст сообщения";
    setHTML("bodyNodeEmail", bodyNode);
    //var bodyNode="";
    bodyNode = window.location.host + "/sendAJAX?json={C:2,stat:-=номер кнопки=-}";
    setHTML("bodyNodeSendIRw433", bodyNode);
    //var bodyNode="";
    bodyNode = window.location.host + "/sendAJAX?json={C:3,st:-=код HEX=-}";
    setHTML("bodyNodeIRHex", bodyNode);
    //var bodyNode="";
    bodyNode = window.location.host + "/sendAJAX?json={C:4,st:-=код незашифрованный=-}";
    setHTML("bodyNodeIRRaw", bodyNode);

    bodyNode = window.location.host + "/setDate?DateTime={}";
    setHTML("bodyNodeActualTime", bodyNode);

    bodyNode = window.location.host + "/ws2811AJAX?json={\"from\":[0],\"to\":[88],\"type\":[2],\"dir\":[0],\"col\":['+col+'],\"wh\":[254],\"br_\":[255],\"num\":1,\"sp\":100,\"dr\":255,\"fd\":45,\"fdt\":3,\"br\":12}";
    setHTML("bodyNodeLed", bodyNode);


    //var bodyNode="";
    bodyNode = "<a  href=/function?json={reboot:1}>" + window.location.host + "/function?json={reboot:1}</a>";
    setHTML("bodyNodeReboot", bodyNode);
}

function other_setupLoaded(data) {
    if (data == null) {
        setHTML("bodyNode", "не удалось загрузить другие настройки");
        return;
    }
    dataOther = JSON.parse(data);
    loadBody();
    loadConditons(0);
    loadOtherHelp();
}

function loadConditons(id) {
    if (id < MAX_COND_NUMBER) {
        readTextFile("Condition" + id + ".txt", function (data) {
            if (data == null) {
                loadConditons(id + 1);
                return;
            }
            Conditions[id] = JSON.parse(data);
            loadConditons(id + 1);
        });
    } else {
        setHTMLControlConditions();
    }
}

function setHTMLControlConditions() {
    var count = 0;
    var adress = [];
    for (condID = 0; condID < MAX_COND_NUMBER; condID++) {
        if (Conditions[condID] !== undefined) {
            for (i = 0; i < 10; i++) {
                if (Conditions[condID].bySignal !== undefined) {
                    if ((parseInt(Conditions[condID].bySignal[i]) == 2) || (parseInt(Conditions[condID].bySignal[i]) == 3)) {
                        adress[adress.length] = i;
                        //count++;
                    }
                }
            }
        }
    }

    var bodyNode = "";
    bodyNode +=
        "<table class='table'>" +
        "<tr><td>" +
        "MQTT топик: " +
        "</td><td>" +
        "MQTT топик управление: " +
        "</td><td>" +
        "удаленное управление HTTP: " +
        "</td></tr>";
    for (condID = 0; condID < MAX_COND_NUMBER; condID++) {
        if (Conditions[condID] !== undefined) {
            for (i = 0; i < 10; i++) {
                if (Conditions[condID].bySignal !== undefined) {
                    if ((parseInt(Conditions[condID].bySignal[i]) == 2) || (parseInt(Conditions[condID].bySignal[i]) == 3)) {
                        MQTT_topic = dataOther.deviceID + "/PLUS/" + condID + "/" + i + "\n";
                        MQTT_control = dataOther.deviceID + "/PLUS/" + condID + "/" + i + "/" + "status" + "\n";
                        MQTT_json = window.location.host + "/aRest?Json={C:1,pin:" + condID + ",stat:" + i + ",val:" + Conditions[condID].bySignalPWM[i] + "}\n";
                        bodyNode +=
                            "<tr><td><code>" +
                            MQTT_topic +
                            "</code></td><td>" +
                            MQTT_control +
                            "</td><td><code>" +
                            MQTT_json +
                            "</code></td></tr>";
                    }
                }
            }
        }
    }

    bodyNode += "</table>";
    setHTML("bodyNodeCond", bodyNode);
}

function loadBody() {
    var bodyNode = "";
    bodyNode +=
        "<table class='table' style='width:100%'>" +
        "<tr><td>" +
        "MQTT топик: " +
        "</td><td>" +
        "MQTT топик управление: " +
        "</td><td>" +
        "удаленное управление HTTP: " +
        "</td><td>" +
        "чтение HTTP: " +
        "</td></tr>";
    for (i = 0; i < PinSetup.numberChosed; i++) {
        MQTT_topic = dataOther.deviceID + "/" + PinSetup.descr[i] + "/" + i + "\n";
        MQTT_control = dataOther.deviceID + "/" + PinSetup.descr[i] + "/" + i + "/" + "status" + "\n";
        //status_val = STAT[i] !== undefined ? STAT[i] ^ 1 : PinSetup.defaultVal[i] ^ 1;
        status_val = PinSetup.defaultVal[i] ^ 1;
        //  if (PinSetup.pin[i] !== -1) {
        //     MQTT_json = "<a href=/aRest?Json={stat:" + i + ",pin:"+status_val+"}>" + window.location.host + "/aRest?Json={pin:" + PinSetup.pin[i] + ",val:"+status_val+"}</a>";
        // } else {
        //    MQTT_json = "<a href=/aRest?Json={stat:" + i + ",val:"+status_val+"}>" + window.location.host + "/aRest?Json={stat:" + i + ",val:"+status_val+"}</a>";
        var http_request = "/sendAJAX?json={\"t\":" + i + ",\"v\":" + status_val + "}";
        MQTT_json = "<a href="+ http_request + ">" +window.location.host + http_request + "</a>";

        //}//sendAJAX?json={"t":127,"v":0}
        MQTT_json_read = "<a href=/aRest?Json={stat:" + i + "}>" + window.location.host + "/aRest?Json={stat:" + i + "}</a>";
        bodyNode +=
            "<td>" +
            "<code>" +
            MQTT_topic +
            "</code>" +
            "</td><td>" +
            MQTT_control +
            "</td><td>" +
            "<code>" +
            MQTT_json +
            "</code>" +
            "</td><td>" +
            "<code>" +
            MQTT_json_read +
            "</code>" +
            "</td></tr>";
    }
    bodyNode += "</table>";
    setHTML("bodyNode", bodyNode);
}

function createXmlHttpObject() {
    if (window.XMLHttpRequest) {
        xmlHttp = new XMLHttpRequest();
    } else {
        xmlHttp = new ActiveXObject('Microsoft.XMLHTTP');
    }
    return xmlHttp;
}

function readTextFile(file, callback) {
    //var rawFile = new XMLHttpRequest();
    var xmlHttp = createXmlHttpObject();
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
    if (document.getElementById(ID)) {
        document.getElementById(ID).innerHTML = value; //range
    }
}