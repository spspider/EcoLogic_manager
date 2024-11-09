/**
 * Created by sergey on 24.09.2017.
 */
var Other_setup = {};
var Conditions = [{}];
var Pin_Setup = {};
var reloadPeriod = 10000;
var MAX_COND_NUMBER = 4;
/*
 var sStatus=[];
 var sId=[];
 var sDescr=[];
 var sWidget=[];
 var sTopic=[];
 */
document.addEventListener('DOMContentLoaded', function () {

    makeStartStopButton();
    //load_all_conditions();
    reloadPeriod = get_cookie("reloadPeriod");
    reloadPeriod = reloadPeriod < 500 ? 500 : reloadPeriod;

    loadfile("pin_setup.txt", function (text) {
        createButtons_pin_setup(text);
       // var respond = {"stat": [10, 10, 1024, 838, 107, 1, 1]};

        //RespondCode(JSON.stringify(respond));

        var sendJSON = JSON.stringify({
            't': "status",
            'v': 0
        });

        //sendAJAX(this, sendJSON);

        loadConditons(Cond_load);

    });

}, false);
function ConditionsLoadComlited() {
    loadfile("License", function (text) {
        alert(text);
        setTimeoutLicenseCode();
        //text==1?StartProgram():setTimeoutLicenseCode();
    })
}
function StartProgram() {
    var sendJSON = JSON.stringify({
        't': "status",
        'v': 0
    });
   // var respond = "{\"stat\":[10.0000000000,100.0000000000,100.0000000000,100]}";
    //RespondCode((respond));
    run();


}
var LicenseCodeTimeout;
function setTimeoutLicenseCode() {
    LicenseCodeTimeout = setTimeout(PromtLicense, 10000);
    StartProgram();
}
function PromtLicense() {
    var person = prompt("Please enter your name", "Harry Potter");
    if (person != null) {
        var submit;
        server = "/License?code=" + person;
        send_request(submit, server);
    }
    clearTimeout(LicenseCodeTimeout);
    LicenseCodeTimeout = setTimeout(PromtLicense, 10000);
}
var myChart = [{}];
var DataChart = [{}];

var Chart = [];
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
    var parsetext = {};

    for (i = 0; i < n; i++) {
        parsetext.id = i;
        parsetext.descr = data.descr[i];
        parsetext.widget = data.widget[i];
        parsetext.topic = parsetext.id + "/" + parsetext.widget + "/" + parsetext.id;
        ParseAndCreateButtons(parsetext);
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

function readTextFile(file, callback) {
    var xmlHttp = createXmlHttpObject();
    xmlHttp.overrideMimeType("application/json");
    xmlHttp.open("GET", file, true);
    xmlHttp.onreadystatechange = function () {

        if (xmlHttp.readyState === 4) {
            if (xmlHttp.status === 200) {
                callback(xmlHttp.responseText);
            } else {
                callback(null);
                clearMyTimeout();
            }
            clearTimeout(timeOut_answer);
        }

    }
    xmlHttp.send(null);
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
    //sendStatus();
    var sendJSON = JSON.stringify({
        't': "status",
        'v': 0
    });
    if (running) {
        sendAJAX(this, sendJSON);
        clearTimeout(timeOut);
        timeOut = setTimeout(loadValuesRun_AJAX, reloadPeriod);
        clearTimeout(timeOut_answer);
        timeOut_answer = setTimeout(clearMyTimeout, (reloadPeriod + 10000));//2 сек Для ответа, если ответа нет - соединение потеряно
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
    if (get_cookie("reloadPeriod")) {
        reloadPeriod = get_cookie("reloadPeriod") > 500 ? get_cookie("reloadPeriod") : 500;
    }
    refreshInput.value = reloadPeriod;
    setVal("run_range", reloadPeriod);
    refreshInput.onchange = function (e) {
        var value = parseInt(e.target.value);
        reloadPeriod = (value > 0) ? value : 0;
        e.target.value = reloadPeriod;
    }
}


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

    name = "выкл";
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

    var nameArr = parsetext.sTopic.split("/");
    var nameWidget = Pin_Setup.widget[id];
    var NewStatus;
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
            NewStatus.value = Pin_Setup.descr[id] + " " + setValueName(parsetext.status ^ Pin_Setup.defaultVal[id]);
            NewStatus.className = SetClassName(parsetext.status, Pin_Setup.defaultVal[id]);

            //NewStatus.className ="btn btn-block btn-default";
            break;
        case "range":
        case 3:
            NewStatus.value = parsetext.status;
            textID = sTopic[id] + "/text";
            setHTML(textID, parsetext.status);
            //document.getElementById(textID).innerHTML = parsetext.status;
            break;
        case "progress-bar":
        case 4:
            var progress = (parsetext.status * 100 / 1024);
            NewStatus.style = "width:" + progress + "%";
            //NewStatus.
            textID = sTopic[id] + "/text";
            setHTML(textID, parsetext.status);
            // document.getElementById(textID).innerHTML = parsetext.status;
            break;
        case "simple-btn":
        case 2:
            NewStatus.value = sDescr[id] + " " + setValueName(parsetext.status ^ Pin_Setup.defaultVal[id]);
            NewStatus.className = SetClassName(parsetext.status, Pin_Setup.defaultVal[id]);
            //NewStatus.className ="btn btn-block btn-default";
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
        case deault:
            NewStatus.value = sDescr[id] + " " + "unknown";

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

function setHTML(ID, value) {
    if (document.getElementById(ID)) {
        document.getElementById(ID).innerHTML = value; //range
    }
}

function getHTML(ID) {
    var value;
    if (document.getElementById(ID)) {
        value = document.getElementById(ID).innerHTML; //range
        return value;
    }
    return undefined;
}


function ParseAndCreateButtons(parsetext) {

    setHTML("demo", getHTML("demo") + "</br>");
    sTopic(parsetext.id, parsetext.topic);
    sDescr(parsetext.id, parsetext.descr);
    sWidget(parsetext.id, parsetext.widget);
    sId(parsetext.topic, parsetext.id);
    var id = parsetext.id;
    var makeWidjet = "";
    /*
     inputWidjet[0] = 'unknown';
     inputWidjet[1] = 'toggle';
     inputWidjet[2] = 'simple-btn';
     inputWidjet[3] = 'range';
     inputWidjet[4] = 'small-badge';
     inputWidjet[5] = 'chart';
     */
    textID = parsetext.topic + "/text";

    switch (parsetext.widget) {
        case 'toggle':
        case 1:
            /*
             var newNode = document.createElement('button');
             newNode.className = 'btn btn-block btn-success';
             newNode.setAttribute("id", parsetext.id);
             newNode.onclick = function() {
             sendNewValue(parsetext.id);
             return false;
             };
             var t = document.createTextNode(parsetext['descr']);
             newNode.appendChild(t);
             document.getElementById("demo").appendChild(newNode);
             */
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

    makeWidjet += "<div id = 'PlusMinus" + id + "'></div>";
    if (makeWidjet) {
        setHTML("demo", getHTML("demo") + makeWidjet);//" pin: "+Pin_Setup.pin[id]+" "+
    }

}
var Cond_load = 0;

function loadConditons(id) {

    if (id < MAX_COND_NUMBER) {
        loadfile("Condition" + id + ".txt", LoadFileComplitedButtons);
    } else {
        StartProgram();
    }


}
function LoadFileComplitedButtons(text) {

    if (text === null) {
        loadConditons(++Cond_load);
        return;
    }
    Conditions[text.ID] = text;
    Data = text;

    /*
     var rootObject = document.getElementById("PlusMinus"+Data.ID);
     var btn = document.createElement("BUTTON");
     var btnP = document.createElement("BUTTON");
     var t = document.createTextNode("-");
     var tP = document.createTextNode("+");
     var status_now = document.createTextNode("statusNow"+Data.ID);
     btn.appendChild(t);
     btnP.appendChild(tP);
     rootObject.appendChild(btn);
     rootObject.appendChild(status_now);
     rootObject.appendChild(btnP);
     */
    var count = 0;
    var adress = [];
    for (i = 0; i < 10; i++) {
        if ((parseInt(Data.bySignal[i]) == 2) || (parseInt(Data.bySignal[i]) == 3)) {
            adress[count] = i;
            count++;
        }

    }
    var textNode = "";
    for (i = 0; i < count; i++) {
        textNode += "<div class='btn-group btn-group-justified'>";
        textNode += Pin_Setup.descr[Data.ID] + " " + i + ": <input id='Plus" + Data.ID + adress[i] + "' class='btn btn-group   btn-default' type='button' value='+' onclick='PlusMinusPress(" + adress[i] + "," + Data.ID + ",true," + Data.bySignalPWM[adress[i]] + ");' />";
        textNode += "<div  id = 'plusMinusValue" + Data.ID + adress[i] + "' class='btn-group' >" + Data.bySignalPWM[adress[i]] + "</div>";
        //textNode +="test";
        textNode += "<input id='Minus" + Data.ID + adress[i] + "' class='btn btn-group btn-default' type='button' value='-' onclick='PlusMinusPress(" + adress[i] + "," + Data.ID + ",false," + Data.bySignalPWM[adress[i]] + ");' />";
        textNode += "</div><br>";
    }
    setHTML("PlusMinus" + Data.ID, textNode);
    //alert(Data.ID);
    //loadConditons();

    //loadConditons(Cond_load++);
    loadConditons(++Cond_load);
}
function SaveCond(id) {

    saveData("Condition" + Conditions[id].ID + ".txt", Conditions[id]);
}
var timeOut_saveCond;
function PlusMinusPress(adress, id, isPlus, value) {
    var value = parseInt(Conditions[id].bySignalPWM[adress]);
    isPlus ? value++ : value--;
    Conditions[id].bySignalPWM[adress] = value;
    setHTML("plusMinusValue" + id + adress, value);
    clearTimeout(timeOut_saveCond);
    timeOut_saveCond = setTimeout(SaveCond, 2000, id);


}

function mouseDownBtn(id, button) {
    //PreVvalue = parseInt(sStatus[id]);
    //PreVvalue = parseInt(Pin_Setup.defaultVal[id] ^ Pin_Setup.defaultVal[id]);
    sStatus[id] = switchToggle(Pin_Setup.defaultVal[id] ^ 1);
    button.className = ('btn btn-block btn btn-danger');
    sendNewValue(button, id);
}

function mouseUpBtn(id, button) {
    //PreVvalue = parseInt(sStatus[id]);
    //PreVvalue = parseInt(Pin_Setup.defaultVal[id] ^1);
    sStatus[id] = (Pin_Setup.defaultVal[id] ^ 1);
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
    return value ^ 1;
}
function sendNewValue(button, id) {
    //topic = id;
    PreVvalue = parseInt(sStatus[id]);
    //NewValue = document.getElementById(sTopic[id]).value;
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

    sendAJAX(button, sendJSON);

}
function sendAJAX(submit, sendJSON) {
    //run();//убрать!! возможны зависания, т.к. sendAjax посылается каждый интервал, указанный в setReloadPeriod
    server = "/sendAJAX?json=" + sendJSON;
    readTextFile(server, RespondCode);
    //send_request(submit, server);
    return false;
}
function send_request(submit, server) {
    var request = createXmlHttpObject();
    request.open("GET", server, true);
    request.send();
    request.timeout = 10000;
    save_status(submit, request);
}

function RespondCode(responseText) {
    //var newString = responseText.replace(/([^\d]*)(\d*)([^\w]*)/, replacer);
    //responseText= responseText.replace(/([*+?^=!:${}()nan|\[\]\/\\])/g, "\$&");
    setHTML("output", (responseText));
    try {
        pasrse = JSON.parse(responseText);
        var newstatus_text = {};
        for (i = 0; i < pasrse.stat.length; i++) {
            newstatus_text.sTopic = i + "/" + Pin_Setup.widget[i] + "/" + i;//parsetext.topic = parsetext.id + "/" + parsetext.widget + "/" + parsetext.id;
            newstatus_text.status = parseInt(pasrse.stat[i].toString());
            newstatus_text.id = i;
            newstatus_text.widjet = Pin_Setup.widget[i];
            SetNewStatus(newstatus_text);
        }
    }
    catch (e) {
        setHTML("output", getHTML("output") + e);
    }

}
function setVal(ID, value) {
    var object;
    if (document.getElementById(ID)) {
        object = document.getElementById(ID);
        if (object.type == "checkbox") {
            document.getElementById(ID).checked = value;
        } else {
            document.getElementById(ID).value = value;
        }
    } else {
        if (document.getElementById("test")) {
        }
    }
}

function save_status(submit, request) {
    old_submit = submit.value;
    request.ontimeout = function (e) {
        //clearMyTimeout();
    };
    request.onreadystatechange = function () {
        if (xmlHttp.readyState === 4) {
            if (xmlHttp.status === 200) {
                RespondCode(request.responseText);
            } else {
                //RespondCode(JSON.stringify({"stat": [0]}));
                clearMyTimeout();
            }
            clearTimeout(timeOut_answer);
        }
        //submit.value = old_submit;
        /*
         request.setTimeout(function () {
         submit.value = old_submit;
         submit_disabled(false);
         }, 1000);
         */
    }
    //submit_disabled(true);
}
function saveData(filename, data) {

    var xmlHttp = createXmlHttpObject();

    var file = new Blob([JSON.stringify(data, null, 2)], {type: "text/plain;charset=utf-8"});
    var a = new FormData();
    a.append("data", file, filename);
    xmlHttp.open("POST", "/edit");
    xmlHttp.send(a);

    xmlHttp.onreadystatechange = function () {

        if (xmlHttp.readyState == 4) {
            if (xmlHttp.status != 200) alert("ERROR[" + xmlHttp.status + "]: " + xmlHttp.responseText);
            else {
                //alert(xmlHttp.responseText);

            }
        }


    }
    xmlHttp.onloadend = function () {

    }
}
function set_cookie(name, value, exp_y, exp_m, exp_d, path, domain, secure) {
    var cookie_string = name + "=" + escape(value);

    if (exp_y) {
        var expires = new Date(exp_y, exp_m, exp_d);
        cookie_string += "; expires=" + expires.toGMTString();
    }
    if (path)
        cookie_string += "; path=" + escape(path);
    if (domain)
        cookie_string += "; domain=" + escape(domain);
    if (secure)
        cookie_string += "; secure";
    document.cookie = cookie_string;
}
function get_cookie(cookie_name) {
    var results = document.cookie.match('(^|;) ?' + cookie_name + '=([^;]*)(;|$)');
    if (results)
        return ( unescape(results[2]) );
    else
        return null;
}
function getVal(ID) {
    var value = -1;
    var object;
    if (document.getElementById(ID)) {
        object = document.getElementById(ID);
        if (object.type == "checkbox") {
            value = document.getElementById(ID).checked;
            //alert(value);
        } else {
            value = document.getElementById(ID).value; //range
        }
    } else {
        if (document.getElementById("test")) {
            //  document.getElementById("test").innerHTML += "<br>wrong:'" + ID + "'"; //range
        }
    }
    return value;
}
