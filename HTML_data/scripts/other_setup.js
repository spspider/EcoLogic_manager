var set_real_time;
var xmlHttp = createXmlHttpObject();

//////////for-json

function toJSONString(form) {
    var obj = {};
    var elements = form.querySelectorAll("input, select, textarea");
    for (var i = 0; i < elements.length; ++i) {
        var element = elements[i];
        var name = element.name;
        var value = element.value;

        if (name) {
            obj[name] = value;
        }
    }

    return JSON.stringify(obj);
}

document.addEventListener("DOMContentLoaded", function () {
    setHTML("btmBtns", bottomButtons());
    load();

    SendTime_onload();
    //onLoad();
});

/*
 function onLoad() {
 var form = document.getElementById("form");
 var output = document.getElementById("output");
 form.addEventListener("submit", function (e) {
 e.preventDefault();
 var json = "jsonArray="+toJSONString(this);
 output.innerHTML = json;
 xmlHttp.open("POST", '/other_setup', true);
 xmlHttp.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
 xmlHttp.send(json_upload);
 alert(json);
 }, false);
 }
 */
function CatchForm() {
    var form = document.getElementById("form");
    var output = document.getElementById("output");
    var JsonString = toJSONString(form);

    var JsonStringParse = JSON.parse(JsonString);
    JsonStringParse.iot_enable = getVal("iot_enable");
    JsonStringParse.geo_enable = getVal("geo_enable");
    JsonStringParse.wifi_scan = getVal("wifi_scan");
    JsonStringParse.ir_loop = getVal("ir_loop");
    JsonStringParse.loop_433 = getVal("loop_433");
    JsonStringParse.ws8211_loop = getVal("ws8211_loop");
    JsonStringParse.save_stat = getVal("save_stat");
    JsonStringParse.PWM_frequency = getVal("PWM_frequency");
    JsonStringParse.IR_recieve = getVal("IR_recieve");
    if (JsonStringParse.emaillogin) {
        JsonStringParse.emaillogin = b64EncodeUnicode(JsonStringParse.emaillogin);
    }
    if (JsonStringParse.password_email) {
        JsonStringParse.password_email = b64EncodeUnicode(JsonStringParse.password_email);
    }
    var json = "jsonArray=" + JSON.stringify(JsonStringParse, null, 2);
    output.innerHTML = json;


    saveData("other_setup.txt", JsonStringParse);
    /*
     xmlHttp.open("POST", '/other_setup', true);
     xmlHttp.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
     xmlHttp.send(json);

     xmlHttp.onloadend = function () {
     setHTML("output", json);
     };
     xmlHttp.onreadystatechange = function () {
     if (xmlHttp.readyState === 4 && xmlHttp.status == "200") {
     //callback(xmlHttp.responseText);
     alert(xmlHttp.responseText);
     }
     }
     */
    //alert(json);
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
                alert(xmlHttp.responseText);

            }
        }


    }
    xmlHttp.onloadend = function () {

    }
}

function handleServerResponse() {
    clearTimeout(set_real_time);
    var res = jsonResponse.time.split(":");
    real_time(hours = res[0], min = res[1], sec = res[2]);
    document.body.style.backgroundColor = "rgb(" + jsonResponse.rgb + ")";
}

//function real_time(hours, min, sec) {
function real_time(JsonTime) {
    var hours = JsonTime.h;
    var min = JsonTime.m;
    var sec = JsonTime.s;
    sec = Number(sec) + 1;
    if (sec >= 60) {
        min = Number(min) + 1;
        sec = 0;
    }
    if (min >= 60) {
        hours = Number(hours) + 1;
        min = 0;
    }
    if (hours >= 24) {
        hours = 0
    }
    JsonTime.h = hours;
    JsonTime.m = min;
    JsonTime.s = sec;
    setHTML("time", hours + ":" + min + ":" + sec);
    setHTML("date", JsonTime.d + "." + JsonTime.n + "." + JsonTime.y);
    //document.getElementById("time").innerHTML = hours + ":" + min + ":" + sec;
//set_real_time = setTimeout("real_time(" + hours + "," + min + "," + sec + ");", 1000);
    set_real_time = setTimeout("real_time(" + JSON.stringify(JsonTime) + ");", 1000);

    //set_real_time = setTimeout("real_time(" +JsonTime+");", 1000);
    //set_real_time = setTimeout(real_time(hours,min,sec), 1000);
}

function sendEmail(submit) {
    server = "/sendEmail?Email='test Email'";
    send_request(submit, server);
    return false;
}

function restart(submit, texts) {
    if (confirm(texts)) {
        server = "/restart?device=ok";
        send_request(submit, server);
        return true;
    } else {
        return false;
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


function SetDate(submit) {
    var date = new Date();
    var day = date.getDate();
    var month = date.getMonth() + 1;
    var year = date.getFullYear();
    if (month < 10) month = "0" + month;
    if (day < 10) day = "0" + day;
    var today = year + "-" + month + "-" + day;
    var time = date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds();
    document.getElementById('BrowserMyDate').value = today;
    document.getElementById('BrowserMyTime').value = time;

    //alert(date);
    //var nowTime = date.getTime();

    //setTimeout(SetDate(), 1000);
    SendTime(submit);
}

function SendTime(submit) {
    //today = document.getElementById('BrowserMyDate').value;
    //time = document.getElementById('BrowserMyTime').value;
    //var someDate = new Date(time);
    //var time = Math.round(new Date().getTime() / 1000);

    //SendTime_onload();

}

function SendTime_onload() {

    var newDate = new Date();
    var time = newDate.getHours() * 60 + newDate.getMinutes();
    //server = "/setDate?DateTime=" + time;
    var jsonStr2 = {};
    jsonStr2["n"] = newDate.getMonth() + 1;
    jsonStr2["d"] = newDate.getDate();
    jsonStr2["y"] = newDate.getFullYear();
    jsonStr2["hm"] = time;
    jsonStr2["h"] = newDate.getHours();
    jsonStr2["m"] = newDate.getMinutes();
    json = JSON.stringify(jsonStr2);
    //document.getElementById("testJSON").innerHTML = JSON.stringify(jsonStr2);
    var jsonPretty = JSON.stringify(JSON.parse(json), null, 2);
    var json_upload = "?DateTime=" + json;
    //json_upload.append("Number", btnId);
    readTextFile("/setDate" + json_upload, function (readback) {
        try {
            var timeReadBack = JSON.parse(readback);
//            real_time(timeReadBack.h, timeReadBack.m, timeReadBack.s);
            real_time(timeReadBack);
            setHTML("output", readback);
            setVal("timezone", timeReadBack.t);
        } catch (e) {
            var timeReadBack = {
                h: 0,
                m: 0,
                s: 0,
                t: 0,
                d: 0,
                n: 0,
                y: 0
            }
            real_time(timeReadBack);
            setVal("timezone", timeReadBack.t);
            //////////////////////////
        }
    });
    /*
     var xmlHttp = createXmlHttpObject();
     xmlHttp.open("POST", '/setDate', true);
     xmlHttp.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
     xmlHttp.send(json_upload);
     xmlHttp.onloadend = function () {
     //setHTML("output", json);
     };
     xmlHttp.onreadystatechange = function () {
     if (xmlHttp.readyState === 4 && xmlHttp.status == "200") {
     callback(xmlHttp.responseText);
     if (xmlHttp.responseText="TimeSetted"){
     alert("новое время установлено!");
     }
     //alert(xmlHttp.responseText);
     }
     }
     */

    //setHTML("test", json);
    //send_request(submit, server);
}

function setTimeBrowser(submit) {
    //currentTime = new Date();
    //time = currentTime.getTime();
    //hours = currentTime.getHours();
    //BrowserMyDate
    //alert(time);
    document.getElementsById('BrowserMyTime').value = "10:20:00";
    server = "/setTime?Time='10:20:00'";
    send_request(submit, server);
}

function load() {

    try {
        readTextFile("other_setup.txt", function (settings) {
            try {
                setHTML("input", getHTML("input") + settings);
                //jsonResponse = jsonResponse !== "" ? $.parseJSON(settings) : {};
                jsonResponse = JSON.parse(settings);
                loadBlock(settings);
            } catch (e) {
                setHTML("input", getHTML("input") + e);
            }
        });
    } catch (e) {
        setHTML("input", getHTML("input") + e);
    }

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
        } else {
            //callback(null);
        }
    }
    xmlHttp.send(null);
}

/*
 function load2() {
 if (xmlHttp.readyState == 0 || xmlHttp.readyState == 4) {
 xmlHttp.open('PUT', 'settings.txt', true);

 xmlHttp.send(null);
 xmlHttp.onload = function (e) {
 jsonResponse = JSON.parse(xmlHttp.responseText);
 loadBlock();

 }
 }
 }
 */
function b64EncodeUnicode(str) {
    // first we use encodeURIComponent to get percent-encoded UTF-8,
    // then we convert the percent encodings into raw bytes which
    // can be fed into btoa.
    return btoa(encodeURIComponent(str).replace(/%([0-9A-F]{2})/g,
        function toSolidBytes(match, p1) {
            return String.fromCharCode('0x' + p1);
        }));
}

function b64DecodeUnicode(str) {
    // Going backwards: from bytestream, to percent-encoding, to original string.
    return decodeURIComponent(atob(str).split('').map(function (c) {
        return '%' + ('00' + c.charCodeAt(0).toString(16)).slice(-2);
    }).join(''));
}

function TestIOT(button) {

}

function loadBlock(settings) {
    data2 = JSON.parse(settings);
    if (data2) {
        if (data2.emaillogin) {
            data2.emaillogin = b64DecodeUnicode(data2.emaillogin);
        }
        if (data2.password_email) {
            data2.password_email = b64DecodeUnicode(data2.password_email);
        }
        //data2.iot_enable

    }

    data = document.getElementsByTagName('body')[0].innerHTML;
    var new_string;
    for (var key in data2) {
        new_string = data.replace(new RegExp('{{' + key + '}}', 'g'), data2[key]);
        data = new_string;
    }
    document.getElementsByTagName('body')[0].innerHTML = new_string;
    setVal("iot_enable", data2.iot_enable);
    setVal("geo_enable", data2.geo_enable);
    setVal("wifi_scan", data2.wifi_scan);
    setVal("ir_loop", data2.ir_loop);
    setVal("loop_433", data2.loop_433);
    setVal("ws8211_loop", data2.ws8211_loop);
    setVal("save_stat", data2.save_stat);
    setVal("PWM_frequency", data2.PWM_frequency);
    setVal("IR_recieve", data2.IR_recieve);
    setHTML("PWMfreq", parseInt(data2.PWM_frequency * 100));
    //handleServerResponse();
    //onLoad();
}

function FreqChange() {
    setHTML("PWMfreq", parseInt(getVal("PWM_frequency")) * 100);
    /*
    readTextFile("function?json={\"PWM_function\":" + parseInt(getVal("PWM_frequency")) + "}", function (callback) {
        //var data = JSON.parse(callback);
        //document.getElementById("output").appendChild(alert_message(JSON.stringify(callback)));
        setHTML("PWMfreq", callback*100);
        //setHTML("NextRepeat0", time);
    })

     */
}

function send_request(submit, server) {
    request = new XMLHttpRequest();
    request.open("GET", server, true);
    request.send();
    save_status(submit, request);
}

function save_status(submit, request) {
    old_submit = submit.value;
    request.onreadystatechange = function () {
        if (request.readyState != 4) return;
        submit.value = request.responseText;
        setTimeout(function () {
            submit.value = old_submit;
            submit_disabled(false);
        }, 1000);
    }
    submit.value = 'Подождите...';
    submit_disabled(true);
}

function submit_disabled(request) {
    var inputs = document.getElementsByTagName("input");
    for (var i = 0; i < inputs.length; i++) {
        if (inputs[i].type === 'submit') {
            inputs[i].disabled = request;
        }
    }
}

function toggle(target) {
    var curVal = document.getElementById(target).className;
    document.getElementById(target).className = (curVal === 'hidden') ? 'show' : 'hidden';
}

function setHTML(ID, value) {
    if (document.getElementById(ID)) {
        document.getElementById(ID).innerHTML = value; //range
    } else {
        if (document.getElementById("test")) {
            document.getElementById("test").innerHTML = "wrong_setHTML:'" + ID + "' value:" + value; //range
        }
    }
}

function getHTML(ID) {
    var value;
    if (document.getElementById(ID)) {
        value = document.getElementById(ID).innerHTML; //range
        return value;
    } else {
        if (document.getElementById("test")) {
            document.getElementById("test").innerHTML = "wrong_getHTML:'" + ID + "'"; //range
        }
    }
    return undefined;
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

function setVal(ID, value) {
    var object;

    if (value !== undefined) {
        //alert(value);
        if (document.getElementById(ID)) {
            object = document.getElementById(ID);
            if (object.type == "checkbox") {
                document.getElementById(ID).checked = value;
            } else {
                document.getElementById(ID).value = value;
            }
        } else {
            if (document.getElementById("test")) {
                //document.getElementById("test").innerHTML += "<br>wrong_setVal:'" + ID + "' value:" + value; //range
            }
        }
    }
}


