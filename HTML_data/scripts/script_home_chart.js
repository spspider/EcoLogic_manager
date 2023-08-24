var Other_setup = {};
var Conditions = [{}];

document.addEventListener('DOMContentLoaded', function () {

    //makeChart("ADC");
    loadBootstap();

    makeStartStopButton();

    //load1();
    //load2();
    //;
    load_all_conditions();
    loadfile("other_setup.txt", function (text) {
        Other_setup = text;
        //setHTML("input",Other_setup.deviceID);
    });

    loadfile("pin_setup.txt", function (text) {
        createButtons_pin_setup(text);
    });

}, false);
if (document.readyState === 'complete') {
    loadBootstap();
}
function loadBootstap(){
    var res = document.createElement("link");
    res.rel = "preload";
    res.as = "style";
    res.href = "https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta/css/bootstrap.min.css";
    document.head.appendChild(res);
}
if (document.readyState === 'complete') {
    loadBootstap();
}
var myChart = [{}];
var DataChart = [{}];

/*
 for (var i = 0; i < 20; i++) {
 DataChart[i] = {
 labels: [],
 series: [
 []
 ]
 };
 }
 */
var Chart = [];
var Graph = [{}];
var timeOut;
var timeOut_answer;
var reloadPeriod = 10000;
var running = false;


///все глобальные переменные до соединения с вебсокетом
var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
//var sTopic=new Array;
//document.getElementById("test")=time();

connection.onopen = function () {
    setHTML("demo", "");
    //setHTML("charts","");
    var myNode = document.getElementById("charts");
    while (myNode.firstChild) {
        myNode.removeChild(myNode.firstChild);
    }

    connection.send("HELLO");
    setTimeout(run, 30000);
    check_if_alive();

    //loadValues();
};

function clearMyTimeout() {
    clearTimeout(timeOut); //останавливаем слудующий таймер, так как этот таймер не остановлен
    running = false;
    alert("timeout!");
}
/*
 function loadValues() {

 if (connection.readyState === 1) {
 //connection.send("STATUS");
 if (timeOut === null) {
 timeOut = setTimeout(loadValues, 10000);

 } else {
 connection.send("STATUS");
 clearTimeout(timeOut);
 clearTimeout(timeOut_answer);
 timeOut = setTimeout(loadValues, 10000);
 timeOut_answer = setTimeout(clearMyTimeout, 12000);
 }
 //setHTML("testConnection", getHTML("testConnection") + "conn:" + getDateTime());
 } else {
 clearTimeout(timeOut);
 }

 //setHTML("demo",getHTML("demo")+"loadValues()");
 }
 */
function load_all_conditions() {
    i = 0;
    loadfile("Condition" + i.toString() + ".txt", function (data) {
        Conditions[i] = data;
        //alert(i+Conditions[i].tID);
    });




}
function createButtons_pin_setup(data) {
    var n = data.numberChosed;
    var parsetext = {};
    for (i = 0; i < n; i++) {
        parsetext.id = data.id[i];
        parsetext.descr = data.descr[i];
        parsetext.widget = data.widget[i];
//        parsetext.topic = Other_setup.prefix + "/" + Other_setup.deviceID + "/" + parsetext.descr + parsetext.id;
        parsetext.topic = parsetext.id + "/" + parsetext.widget + "/" + parsetext.id;
        ParseAndCreateButtons(parsetext);
    }

    var newstatus_text = {};
    newstatus_text.sTopic = "2/3/2";
    newstatus_text.status = 100;
    SetNewStatus(newstatus_text);


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
        if (xmlHttp.readyState === 4 && xmlHttp.status == "200") {
            callback(xmlHttp.responseText);
        }
    }
    xmlHttp.send(null);
}

function loadValuesRun() {
    if (!running) return;
    //sendStatus();
    if (connection.readyState === 1) {
        connection.send("STATUS");
    }
    if (running) {
        clearTimeout(timeOut_answer);
        setTimeout(loadValuesRun, reloadPeriod);
        timeOut_answer = setTimeout(clearMyTimeout, (reloadPeriod + 2000));//2 сек Для ответа, если ответа нет - соединение потеряно
    }
}
function run() {
    if (!running) {
        running = true;
        loadValuesRun();
    }
}
function makeStartStopButton() {
    var refreshInput = document.getElementById("refresh-rate");
    refreshInput.value = reloadPeriod;
    refreshInput.onchange = function (e) {
        var value = parseInt(e.target.value);
        reloadPeriod = (value > 0) ? value : 0;
        e.target.value = reloadPeriod;
    }
    var stopButton = document.getElementById("stop-button");
    stopButton.onclick = function (e) {
        running = false;
    }
    var startButton = document.getElementById("start-button");
    startButton.onclick = function (e) {
        run();
    }
}
function check_if_alive() {
    if (connection.readyState === connection.CLOSED) {
        location.reload();
    }
    if ((!connection)) {
        alert("Connection lost");
        location.reload();
    }
    //alert()
    setTimeout(check_if_alive, 30000);
}
connection.onerror = function (error) {

    console.log('WebSocket Error ', error);
    //document.getElementById("demo").innerHTML += 'WebSocket Error ' + error;
    setHTML("demo", getHTML("demo") + 'WebSocket Error ' + error);
    //location.reload();
};

connection.onmessage = function (evt) {


    ParseText(evt.data);

    //setTimeout(loadValues(), 50000);
};

function ParseText(in_parsetext) {
    if (in_parsetext) {
        try {
            parsetext = JSON.parse(in_parsetext);
        } catch (e) {
            splitStringConfig(parsetext);
            //splitStringConfig()
            //document.getElementById("demo").innerHTML += "//" + e + "//";
            return;

        }
    }
    if (parsetext) {
        if (parsetext.sTopic) {
            SetNewStatus(parsetext);
            clearTimeout(timeOut_answer); //удаляем таймер ответа,т.к. ответ пришел

        } else if (parsetext.topic) {
            ParseAndCreateButtons(parsetext);

        }
    }
}

function splitStringConfig(StrConfig) {
    var splittedString = StrConfig.split("/");
    if (splittedString[1] == "IoTmanager") {
        for (i = 0; i < StrConfig.length; i++) {
            if (splittedString[i] == null) {
                break;
            }
            setHTML("demo", getHTML("demo") + "/" + splittedString[i]);
            //document.getElementById("demo").innerHTML += "/" + splittedString[i];
            if (splittedString[i] == "config") {
                break;
            }
        }
    }
}

function sendStatus() {
    //setHTML("input",Other_setup.mqttpass);
    var jsonRecieveArrStatus = new Array();
    var random = Math.floor(Math.random() * (100 - 10)) + 10;
    jsonRecieveArrStatus[0] = '{"sTopic":"/IoTmanager/dev01-kitchen/light0","status":"0"}';
    jsonRecieveArrStatus[1] = '{"sTopic":"/IoTmanager/dev01-kitchen/light1","status":"1"}';
    jsonRecieveArrStatus[2] = '{"sTopic":"/IoTmanager/dev01-kitchen/dim-light","status":"1023"}';
    jsonRecieveArrStatus[3] = '{"sTopic":"/IoTmanager/dev01-kitchen/ADC","status":' + random + '}';
    jsonRecieveArrStatus[4] = '{"sTopic":"/IoTmanager/dev01-kitchen/light4","status":"1"}';
    jsonRecieveArrStatus[5] = '{"sTopic":"/IoTmanager/dev01-kitchen/red","status":"0"}';
    jsonRecieveArrStatus[6] = '{"sTopic":"/IoTmanager/dev01-kitchen/green","status":"0"}';
    jsonRecieveArrStatus[7] = '{"sTopic":"/IoTmanager/dev01-kitchen/blue","status":' + random + '}';
    for (i = 0; i < jsonRecieveArrStatus.length; i++) {
        //SetNewStatus(jsonRecieveArrStatus[i]);
        ParseText(jsonRecieveArrStatus[i])
    }

}

function SetClassName(newstatus) {
    var NewclassName = "btn btn-block btn-default";
    if (newstatus == "1") {
        NewclassName = "btn btn-block btn-success";
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

function SetNewStatus(Statusmessage) {
    parsetext = Statusmessage;
    var id = sId[parsetext.sTopic];

    var nameArr = sTopic[id].split("/");
    var nameWidget = nameArr[nameArr.length - 1] + id;
    //alert(nameChart);
    //alert(nameWidget);
    //setHTML("demo", getHTML("demo") + "<br>lookfor:" + nameWidget);

    var NewStatus;
    if (document.getElementById(parsetext.sTopic)) {
        NewStatus = document.getElementById(parsetext.sTopic);
    } else if (document.getElementById(nameWidget)) {
        NewStatus = document.getElementById(nameWidget);

    } else {
        return;
    }

    /*
     inputWidjet[0] = 'unknown';
     inputWidjet[1] = 'toggle';
     inputWidjet[2] = 'simple-btn';
     inputWidjet[3] = 'range';
     inputWidjet[4] = 'small-badge';
     inputWidjet[5] = 'chart';
     */
    switch (sWidget[id]) {
        case "toggle":
        case 1:
            NewStatus.value = sDescr[id] + " " + setValueName(parsetext.status);
            NewStatus.className = SetClassName(parsetext.status);
            //NewStatus.className ="btn btn-block btn-default";
            break;
        case "range":
        case 3:
            NewStatus.value = parsetext.status;
            textID = sTopic[id] + "/text";
            setHTML(textID, parsetext.status);
            //document.getElementById(textID).innerHTML = parsetext.status;
            break;
        case "small-badge":
        case 4:
            var progress = (parsetext.status * 100 / 1024);
            NewStatus.style = "width:" + progress + "%";
            textID = sTopic[id] + "/text";
            setHTML(textID, parsetext.status);
            // document.getElementById(textID).innerHTML = parsetext.status;
            break;
        case "simple-btn":
        case 2:
            NewStatus.value = sDescr[id] + " " + setValueName(parsetext.status);
            NewStatus.className = SetClassName(parsetext.status);
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
        case deault:
            NewStatus.value = sDescr[id] + " " + "unknown";
    }
    //NewStatus.value=sDescr[id]+" "+parsetext.status;
    sStatus(id, parsetext.status);
}

function isEmpty(obj) {
    for (var prop in obj) {
        if (obj.hasOwnProperty(prop))
            return false;
    }

    return true;
}

function getDateTime() {
    var now = new Date();
    var year = now.getFullYear();
    var month = now.getMonth() + 1;
    var day = now.getDate();
    var hour = now.getHours();
    var minute = now.getMinutes();
    var second = now.getSeconds();
    if (month.toString().length == 1) {
        var month = '0' + month;
    }
    if (day.toString().length == 1) {
        var day = '0' + day;
    }
    if (hour.toString().length == 1) {
        var hour = '0' + hour;
    }
    if (minute.toString().length == 1) {
        var minute = '0' + minute;
    }
    if (second.toString().length == 1) {
        var second = '0' + second;
    }
    //var dateTime = year+'/'+month+'/'+day+' '+hour+':'+minute+':'+second;
    var dateTime = hour + ':' + minute + ':' + second;
    return dateTime;
}

function makeChart(name, id) {
    var options = {
        seriesBarDistance: 100
    };
    Chart[id] = new Chartist.Line(name, DataChart[id], options);
}

function updateChart(name, id) {

    Chart[id].update(DataChart[id]);
    //Chart[id].update.bind(DataChart[id])
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

function sStatus(id, status) {
    sStatus[id] = status;
}

function sendText() {

    var stringConf = "/IoTmanager/dev01-kitchen/config";
    splitStringConfig(stringConf);
    var jsonRecieveArr = new Array();
    // Construct a msg object containing the data the server needs to process the message from the chat client.
    jsonRecieveArr[0] = '{"id":"0","page":"Kitchen","descr":"Light-0","widget":"simple-btn","topic":"/IoTmanager/dev01-kitchen/light0","color":"blue"}';
    //document.getElementById("demo").innerHTML += " id:"+JSON.stringify(jsonRecieve);
    jsonRecieveArr[1] = '{"id":"1","page":"Kitchen","descr":"Light-1","widget":"toggle","topic":"/IoTmanager/dev01-kitchen/light1","color":"orange"}';
    jsonRecieveArr[2] = '{"id":"2","page":"Kitchen","descr":"Dimmer","widget":"range","topic":"/IoTmanager/dev01-kitchen/dim-light","style":"range-calm","badge":"badge-assertive","leftIcon":"ion-ios-rainy-outline","rightIcon":"ion-ios-rainy"}';
    jsonRecieveArr[3] = '{"id":"3","page":"Kitchen","descr":"ADC","widget":"chart","topic":"/IoTmanager/dev01-kitchen/ADC","badge":"badge-balanced"}';
    jsonRecieveArr[4] = '{"id":"4","page":"Outdoor","descr":"Gardenlight","widget":"chart","topic":"/IoTmanager/dev01-kitchen/light4","color":"red"}';
    jsonRecieveArr[5] = '{"id":"5","page":"Kitchen","descr":"RED","widget":"range","topic":"/IoTmanager/dev01-kitchen/red","style":"range-assertive","badge":"badge-assertive"}';
    jsonRecieveArr[6] = '{"id":"6","page":"Kitchen","descr":"GREEN","widget":"range","topic":"/IoTmanager/dev01-kitchen/green","style":"range-balanced","badge":"badge-balanced"}';
    jsonRecieveArr[7] = '{"id":"7","page":"Kitchen","descr":"BLUE","widget":"range","topic":"/IoTmanager/dev01-kitchen/blue","style":"range-calm","badge":"badge-calm"}';
    //ParseAndCreateButtons(jsonRecieveArr[7]);
    //document.getElementById("demo").innerHTML += "//";
    for (i = 0; i < jsonRecieveArr.length; i++) {
        var parsedtext = jsonRecieveArr[i]
        //ParseAndCreateButtons(parsedtext);
        ParseText(parsedtext);
        //ParseAndCreateButtons(jsonRecieveArr[1]);
        //document.getElementById("demo").innerHTML += "//";
    }

    length = jsonRecieveArr.length;
    connectonclick(length);

}

function ParseAndCreateButtons(parsetext) {

    //document.getElementById("demo").innerHTML += "</br>";
    setHTML("demo", getHTML("demo") + "</br>");
    sTopic(parsetext.id, parsetext.topic, parsetext.descr);
    sDescr(parsetext.id, parsetext.descr);
    sWidget(parsetext.id, parsetext.widget);
    sId(parsetext.topic, parsetext.id);
    var id = parsetext.id;
    //sTopic[parsetext.id] =parsetext.topic;
    //sId.push(parsetext.id);
    //sDescr.push(parsetext.descr);
    var makeWidjet;
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
            makeWidjet = "<input id='" + parsetext.topic + "' class='btn btn-block btn-success' type='button' value='" + parsetext['descr'] + "' onclick='sendNewValue(" + parsetext.id + ");' /></br>";
            //setHTML("input",Conditions[0].tID);
            break;
        case 'range':
        case 3:
            makeWidjet = parsetext.descr + " <p1 id='" + textID + "'></p1><input id='" + parsetext.topic + "' class='form-control'  type='range' min='0' max='1024' step='1' onchange='sendNewValue(" + parsetext.id + ");'/></br>";
            break;
        case 'small-badge':
        case 4:
            makeWidjet = parsetext.descr + "<div class='progress'><div id='" + parsetext.topic + "' class='progress-bar' role='progressbar' aria-valuenow='0' aria-valuemin='0' aria-valuemax='1024' style='width:100%'><p1 id='" + textID + "'></p1></div></div>";
            break;
        case 'simple-btn':
        case 2:
            makeWidjet = "<input id='" + parsetext.topic + "' class='btn btn-block btn-success' type='button' value='" + parsetext['descr'] + "' onmousedown='mouseDownBtn(" + parsetext.id + ",this)' onmouseup='mouseUpBtn(" + parsetext.id + ",this)' /></br>";
            break;
        case 'chart':
        case 5:
            //makeWidjet = "<canvas id='" + parsetext.topic + "' width='100' height='50'></canvas></br>";
            //name = parsetext.topic.replace(/[&\/\\#,+()$~%.'":*?<>{}]-/g, '');
            var that_topic = parsetext.topic;

            var nameArr = that_topic.split("/");

            var nameChart = nameArr[nameArr.length - 1] + parsetext.id;


            var countup = this;
            var newNode = document.createElement('canvas');
            newNode.className = nameChart;
            newNode.id = nameChart;
            //newNode.innerHTML = "nameChart:" + nameChart;
            document.getElementById("charts").appendChild(newNode);
            //document.getElementById("charts").innerHTML += "<canvas id = 'myChart" + parsetext.id + "'></canvas>";

            //Graph[parsetext.id] = createGraph(document.getElementById(nameChart), parsetext.descr, 100, 100, 0, 1023, false, "cyan");
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

            break;
        default:
            //thatChart = false;
            makeWidjet = "unknown";
    }


    if (makeWidjet) {
        //document.getElementById("demo").innerHTML += makeWidjet;
        setHTML("demo", getHTML("demo") + makeWidjet);
    }

}


function mouseDownBtn(id, button) {
    //button.setClass
    //alert("ok");
    //button.classList.className('btn btn-block btn btn-danger');
    PreVvalue = parseInt(sStatus[id]);
    sStatus[id] = switchToggle(PreVvalue);
    button.className = ('btn btn-block btn btn-danger');
    sendNewValue(id);
    //document.getElementById("test").innerHTML += "1";
}

function mouseUpBtn(id, button) {
    PreVvalue = parseInt(sStatus[id]);
    //NewStatus.className = SetClassName(parsetext.status);
    //PreVvalue = parseInt(sStatus[id]);
    sStatus[id] = switchToggle(PreVvalue);
    if (PreVvalue == 1) {
        button.className = ('btn btn-block btn btn-success');
    }
    if (PreVvalue == 0) {
        button.className = ('btn btn-block btn btn-primary');
    }
    sendNewValue(id);
}

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


function switchToggle(value) {
    switch (value) {
        case 0:
            return 1;
            break;
        case 1:
            return 0;
            break;
        default:
            return 0;
            break;
        //case 1:		value=0; break;
    }
}

function sendNewValue(id) {

    //document.getElementById("demo").innerHTML+=sTopic[id]+"/control";
    topic = sTopic[id] + "/control";
    PreVvalue = parseInt(sStatus[id]);
    NewValue = document.getElementById(sTopic[id]).value;
    //document.getElementById("demo1").innerHTML+=sTopic[id]+NewValue;
    textID = sTopic[id] + "/text";
    if (document.getElementById(textID)) {
        document.getElementById(textID).innerHTML = NewValue
    }
    //document.getElementById("demo1").innerHTML+=text;
    switch (sWidget[id]) {
        case "toggle":
            setvalue = switchToggle(PreVvalue);
            break;
        case "simple-btn":
            setvalue = switchToggle(PreVvalue);
            break;
        case "range":
            setvalue = NewValue;
            break;
        case "chart":
            //setvalue = NewValue;
            break;
    }
    //document.getElementById("demo1").innerHTML+=setvalue;

    //JSON.stringify({ x: 5, y: 6 });
    sendJSON = JSON.stringify({
        'topic': topic,
        'newValue': setvalue
    });
    var docDemo1 = document.getElementById("test");

    //docDemo1.innerHTML += sendJSON;

    connection.send(sendJSON);


}