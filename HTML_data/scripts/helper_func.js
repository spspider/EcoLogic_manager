//var HelperLoaded=true;




function createXmlHttpObject() {
    var xmlHttp;
    if (window.XMLHttpRequest) {
        xmlHttp = new XMLHttpRequest();
    } else {
        xmlHttp = new ActiveXObject('Microsoft.XMLHTTP');
    }
    return xmlHttp;
}


var Activation = -1;

function saveData(filename, data, returnCallback) {
    var xmlHttp = createXmlHttpObject();
    try {
        if (testJson(data)) {
            data = JSON.parse(data);
        }
    } catch (e) {
        returnCallback("JSON Parse Error: " + e.message);
        return;
    }
    var file = new Blob([JSON.stringify(data)], { type: "text/plain;charset=utf-8" });
    var formData = new FormData();
    formData.append("data", file, filename);
    var hasCalledBack = false; // to make sure callback is called only once
    function handleResponse() {
        if (xmlHttp.readyState === 4) {
            if (xmlHttp.status === 200) {
                if (!hasCalledBack) {
                    returnCallback(xmlHttp.responseText);
                    hasCalledBack = true;
                }
            } else {
                if (!hasCalledBack) {
                    returnCallback("ERROR[" + xmlHttp.status + "]: " + xmlHttp.responseText);
                    hasCalledBack = true;
                }
            }
        }
    }
    xmlHttp.onreadystatechange = handleResponse;
    xmlHttp.onloadend = function () {
        if (xmlHttp.status == 200) { // handle other statuses
            returnCallback(xmlHttp.responseText);
            hasCalledBack = true;
        }
    };

    xmlHttp.open("POST", "/edit", true);
    xmlHttp.timeout = 4000;
    xmlHttp.send(formData);
}
function readTextFile(file, callback) {
    var xmlHttp = createXmlHttpObject();
    xmlHttp.overrideMimeType("application/json");
    xmlHttp.open("GET", file, true);
    xmlHttp.timeout = 10000;
    xmlHttp.onreadystatechange = function () {

        if (xmlHttp.readyState === 4) {
            if (xmlHttp.status === 200) {
                callback(xmlHttp.responseText);
            } else {
                //callback(null);
            }
        } else {
            //callback(null);
        }

    };
    xmlHttp.onloadend = function () {
        if (xmlHttp.status === 404) {
            callback(404);
        }
    };


    xmlHttp.send(null);
}

function readTextFiles_array(files, callback) {
    if (files.length === 0) {
        // All files have been read, call final callback function
        callback();
    } else {
        // Read the first file in the array using readTextFile
        var file = files[0];
        readTextFile(file, function (result) {
            if (result === 404) {
                // Handle file not found error
            } else {
                // Handle successful file read
                // Call readTextFiles again with the remaining files in the array
                readTextFiles(files.slice(1), callback);
            }
        });
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
        return (unescape(results[2]));
    else
        return null;
}

function setVal(ID, value) {
    const object = document.getElementById(ID);
    if (object) {
        if (object.type === "checkbox") {
            object.checked = value;
        } else {
            object.value = value;
        }
        return true;
    }
    return false;
}

function setHTML(ID, value) {
    if (document.getElementById(ID)) {
        document.getElementById(ID).innerHTML = value; //range
    }
}

function loadjQuery(url, success) {
    var script = document.createElement('script');
    script.src = url;
    var head = document.getElementsByTagName('head')[0],
        done = false;
    head.appendChild(script);
    // Attach handlers for all browsers
    script.onload = script.onreadystatechange = function () {
        if (!done && (!this.readyState || this.readyState == 'loaded' || this.readyState == 'complete')) {
            done = true;
            success();
            script.onload = script.onreadystatechange = null;
            head.removeChild(script);
        }
    };
}

function loadBootstrap() {
    var element = document.body,
        style = window.getComputedStyle(element),
        top = style.getPropertyValue('color');
    if (top != 'rgb(51, 51, 51)') {
        var el = document.head,
            elChild = document.createElement('link');
        // elChild.innerHTML = '<link sync rel="stylesheet" href="bootstrap.min.css">';
        elChild.innerHTML = '<link rel="stylesheet" type="text/css" href="scripts/style_generated.css">';
        el.insertBefore(elChild, el.firstChild);
    }
}

function getVal(ID) {
    const object = document.getElementById(ID);
    if (object) {
        return object.type === "checkbox" ? object.checked : object.value;
    }
    return -1;
}

function getHTML(ID) {
    var value;
    if (document.getElementById(ID)) {
        value = document.getElementById(ID).innerHTML; //range
        return value;
    } else {
        if (document.getElementById("test")) {
            //document.getElementById("test").innerHTML += "<br>wrong_getHTML:'" + ID + "'"; //range
        }
    }
    return undefined;
}

function createTD(tr, text) {
    var td = document.createElement('td');
    if (typeof (text) === "object") {
        td.appendChild(text);
    } else if (typeof (text) === "string") {
        td.appendChild(document.createTextNode(text));
    }
    tr.appendChild(td);
    return td;
}

function createTR(tr) {
    var Newtr = document.createElement('tr');
    //Newtr.id = "tr" + i;
    Newtr.appendChild(tr);
    return Newtr;
}

function alert_message(message, timeout) {
    if (!timeout) {
        timeout = 10;
    }
    var div = document.createElement('div');
    div.className = "alert alert-success";
    div.innerHTML = message;
    div.id = "ok_message";
    setTimeout(function () {
        div.remove();
    }, timeout * 1000);
    //main_.appendChild(div);
    return div;
}

function testJson(text) {
    if (typeof text !== "string") {
        return false;
    }
    try {
        JSON.parse(text);
        return true;
    } catch (e) {
        return false;
    }
}

function makeinOption(inputOption, id, onChange) {
    var options = "<select class='form-control option' " +
        "" +
        "id=" + id + " onchange='" + onChange + ";'>";

    var i1;
    var inputCorrectedOPtion = [];
    var i2 = 0;

    for (i1 = 0; i1 < inputOption.length; i1++) {
        // inputCorrectedOPtion[i2] = inputOption[i1];
        // i2++;
    }

    for (i1 = 0; i1 < inputOption.length; i1++) {
        if (inputOption[i1] !== null) {
            var option_that = inputOption[i1] !== undefined ? inputOption[i1] : "---";
            options += "<option>" + option_that + "</option>";
            //options.concat(addOpt);
        }
    }
    options += "</select>";
    return options;
}

function makeinOption_child(inputOption, id, onChange) {
    var array = inputOption;
    var selectList = document.createElement("select");
    selectList.className = 'form-control';
    selectList.id = id;
    selectList.onchange = onChange;

    for (var i = 0; i < array.length; i++) {
        var option = document.createElement("option");
        option.value = array[i];
        option.text = array[i];
        selectList.appendChild(option);
    }
    return selectList;
}

function send_code_back() {
    var code = getVal('activation_input');
    readTextFile("/function?json={\"Activation\":\"2\",\"code\":\"" + code + "\"}", function (callback) {
        if (parseInt(callback) === 1) {
            alert("activated:");
        } else {
            alert("wrong code:" + callback);
        }
    });
};


function ActivateDialog() {
    if (document.getElementById('activation_input')) return;
    readTextFile('/function?json={\"Activation\":\"1\"}', function (callback) {
        //var x = prompt("ActivationCode:\n" + callback, "");


        var page = "copy this code=" + (callback);
        var message = "follow this link:\n" +
            "<a href='" + page + "'class='form-control' >" + page + "</a>" +
            "\n then Enter code:\n" +
            "<input type='text' class='form-control' id='activation_input'>" +
            "<input type='button' class='form-control' id='btn_activation' value='OK' onclick='send_code_back()'>";
        var button = document.getElementById("activation_button");
        button.appendChild(alert_message(message, 30));

    });
}


function check_if_activated() {
    var license_code = "<a id ='activation_button' class='btn btn-block btn-default' onclick='ActivateDialog()' type='button'>activate</a>";
    // readTextFile('/function?json={\"Activation\":\"0\"}', function (callback) {//проверить если активирован
    //     if (parseInt(callback) === 1) {//Activated
    //         Activation = 1;
    //         setHTML("activation_button", "");
    //         return 1;

    //     } else {//не активирован
    //         Activation = 0;
    //         setHTML("btmBtns", getHTML("btmBtns") + license_code);
    //         return 0;
    //     }
    // });
    return 1;
}

function bottomButtons() {
    //check_if_activated();
    var btmBtns =
        "<div class='btn-group btn-group-justified'>" +
        "<a class='btn btn-block btn-default' type='button' href='/'>cont</a>" +
        "<a class='btn btn-block btn-default' type='button' href='/wifi'>Wifi</a>" +
        "<a class='btn btn-block btn-default' type='button' href='/other_setup'>conn</a>" +
        "<a class='btn btn-block btn-default' type='button' href='/pin_setup'>buttons</a>" +
        // "<a class='btn btn-block btn-default' href='/IR_setup'>IR</a>" +
        "<a class='btn btn-block btn-default' type='button' href='/condition'>condition</a>" +
        "<a class='btn btn-block btn-default' type='button' href='/ws2811.html'>ws2811</a>" +
        "<a class='btn btn-block btn-default' type='button' href='/help'>?</a>" +
        //       "<a class='btn btn-block btn-default' id = 'activation' onclick='ActivateDialog()' type='button'></a>"+
        " </div>";
    return btmBtns;
}

function bottomButtons2() {
    const btmBtns = document.createElement('div');
    btmBtns.className = 'btn-group btn-group-justified';
    const buttons = [
        { href: '/', text: 'cont' },
        { href: '/wifi', text: 'Wifi' },
        { href: '/other_setup', text: 'conn' },
        { href: '/pin_setup', text: 'buttons' },
        { href: '/condition', text: 'condition' },
        { href: '/ws2811.html', text: 'ws2811' },
        { href: '/help', text: '?' }
    ];
    buttons.forEach(button => {
        const a = document.createElement('a');
        a.className = 'btn btn-block btn-default';
        a.type = 'button';
        a.href = button.href;
        a.textContent = button.text;
        btmBtns.appendChild(a);
    });
    return btmBtns;
}