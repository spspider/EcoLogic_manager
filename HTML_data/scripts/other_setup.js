var set_real_time;

document.addEventListener("DOMContentLoaded", function () {
    loadSettings();
    SendTime_onload();
});

function loadSettings() {
    // Загружаем device_id с Arduino
    setHTML("btmBtns", bottomButtons());
    readTextFile("function?data={\"get_device_id\":1}", function (deviceId) {
        // Загружаем настройки из файла
        readTextFile("other_setup.txt", function (settings) {
            const data = {};

            // Добавляем deviceID
            if (deviceId && deviceId !== '404') {
                data.deviceID = deviceId;
                localStorage.setItem('deviceID', deviceId);
            }

            // Добавляем настройки
            if (settings && settings !== '404') {
                try {
                    const fileData = JSON.parse(settings);
                    Object.assign(data, fileData);
                    localStorage.setItem('otherSetupSettings', settings);
                } catch (e) {
                    console.error('Error parsing settings:', e);
                }
            }

            applySettings(data);
        });
    });
}

function applySettings(data) {
    // Заменяем {{}} в HTML
    let html = document.body.innerHTML;
    Object.keys(data).forEach(key => {
        html = html.replace(new RegExp('{{' + key + '}}', 'g'), data[key] || '');
    });
    document.body.innerHTML = html;

    // Применяем настройки к элементам
    Object.keys(data).forEach(key => {
        const element = document.getElementById(key);
        if (element) {
            if (element.type === 'checkbox') {
                element.checked = data[key];
            } else {
                element.value = data[key] || '';
            }
        }
    });

    if (data.emaillogin) setVal('emaillogin', b64DecodeUnicode(data.emaillogin));
    if (data.password_email) setVal('password_email', b64DecodeUnicode(data.password_email));
    if (data.PWM_frequency) setHTML("PWMfreq", parseInt(data.PWM_frequency));
    
    toggleStaticIP();
}


function CatchForm() {
    const formData = collectFormData();
    const jsonString = JSON.stringify(formData, null, 2);

    // Отображаем JSON для диагностики
    setHTML("output", jsonString);

    localStorage.setItem('otherSetupSettings', jsonString);

    saveData("other_setup.txt", formData, function (response) {
        document.getElementById("output").appendChild(alert_message('Settings saved successfully!', 4));
    });
}

function collectFormData() {
    const form = document.getElementById("form");
    const data = {};

    // Собираем все поля формы
    const elements = form.querySelectorAll("input, select, textarea");
    elements.forEach(element => {
        if (element.name && element.name !== 'deviceID') { // Исключаем deviceID
            if (element.type === 'checkbox') {
                data[element.name] = element.checked;
            } else {
                data[element.name] = element.value;
            }
        }
    });

    // Кодируем пароли
    if (data.emaillogin) {
        data.emaillogin = b64EncodeUnicode(data.emaillogin);
    }
    if (data.password_email) {
        data.password_email = b64EncodeUnicode(data.password_email);
    }

    return data;
}

// Удаляем дублирующую функцию - используем из helper_func.js

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


// Удалено - используем из helper_func.js


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
    var jsonPretty = JSON.stringify(JSON.parse(json), null, 2);
    var json_upload = "?DateTime=" + json;
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

}

function setTimeBrowser(submit) {
    document.getElementsById('BrowserMyTime').value = "10:20:00";
    server = "/setTime?Time='10:20:00'";
    send_request(submit, server);
}

function testMQTTConnection() {
    alert('MQTT test not implemented yet');
}

function FreqChange() {
    setHTML("PWMfreq", parseInt(getVal("PWM_frequency")));
}

function toggleStaticIP() {
    const checkbox = document.getElementById('use_static_ip');
    const fields = document.getElementById('static_ip_fields');
    fields.style.display = checkbox.checked ? 'block' : 'none';
}

function send_request(submit, server) {
    var request = new XMLHttpRequest();
    request.open("GET", server, true);
    request.send();
}

// Удалено - используем из helper_func.js

function TestIOT(button) {

}

// Удаляем loadBlock - заменена на applySettings



// Удаляем дублирующие функции - используем из helper_func.js


