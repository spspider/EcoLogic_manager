document.addEventListener("DOMContentLoaded", load);
var id = 0;
var jsonPinSetup = "{}";
var jsonCondtion = [{}];

var Condition_number = 3;
var Naumber_in_condition = 4;

function load() {
    //alert("alert"+getParameterByName('id'));
    id = getParameterByName('id');
    //id = 1;
    readTextFile("/function?json={'cond_setup':1}", function (callback) {
        //readTextFile("cond_setup.txt", function (callback) {
        if (testJson(callback)) {
            JSON.parse(callback);
            var Data_limits = JSON.parse(callback);
            Condition_number = Data_limits.ConNum;
            Naumber_in_condition = Data_limits.NumCon;
        }else{
            //alert("btnId");
        }
        //alert(Data_limits.NUM_LEDS);
        readPinsetup();
    });
    
    setHTML("btmBtns", bottomButtons());
}

function makeSelect() {
    var add_option = "";
    var result = "";
    for (i = 0; i < Condition_number; i++) {
        add_option += "<option> №:" + i + " " + jsonPinSetup.descr[i] + "</option>";
    }
    result = "<select class='form-control' id='NumberWidget' onchange='makereadCond(i)'>" + add_option + "</select>";
    setHTML("select_condition", result);
    if (id !== null) {
        document.getElementById("NumberWidget").selectedIndex = id; //устанавливаем из страницы
    } else {
        id = 0;
        document.getElementById("NumberWidget").selectedIndex = id;
    }
}

////////pin_setup.txt//////////////////////////////////////
function readPinsetup() {
    var data = {};

    try {
        readTextFile("pin_setup.txt", function (text) {
            if (text === null) {
                readCondition(0);
                makeSelect();
                return;
            }
            try {
                data = JSON.parse(text);
                setHTML("test", getHTML("test") + "<br>" + text);
                readCondition(0);
            } catch (e) {
                document.getElementById("test").innerHTML += e;
            }
            jsonPinSetup = data;
            makeSelect();
        });
    } catch (e) {
    }
}

function makereadCond(CondID) {
    setHTML("table", "");
    makeAddbuttons(-1);
    deleteRow(0);
    var ch_id;
    if (document.getElementById("NumberWidget")) {
        ch_id = document.getElementById("NumberWidget").selectedIndex;
    }
    if (ch_id === null) {
        ch_id = 0;
    }
    var data = {};
    if (jsonCondtion[ch_id]) {
        data = jsonCondtion[ch_id];
    }
    setHTML("input", JSON.stringify(data));
    createCondition(data);
}

function readCondition(CondID) {

    var id = CondID;
    var ch_id = 0;

    if (document.getElementById("NumberWidget")) {
        ch_id = document.getElementById("NumberWidget").selectedIndex;
    }
    var data = {};
    try {
        readTextFile("Condition" + id + ".txt", function (text) {
            try {
                
                data = JSON.parse(text);
                jsonCondtion[id] = data;
                logConditions();
                if (CondID === 3) {
                    createCondition(jsonCondtion[ch_id]);
                }
                //              CondID++;
            } catch (e) {
                //                CondID++;
            }
            if (CondID < 3) {
                CondID++;
                readCondition(CondID);//читаем следующее условие
            }

        });
    } catch (e) {

        //alert("no file");
        //CondID++;
        //setHTML("input", e);

    }

}

function logConditions() {
    var allConditions = "";
    for (i = 0; i < jsonCondtion.length; i++) {
        allConditions += "<br>" + i + ":" + JSON.stringify(jsonCondtion[i]);
    }
    setHTML("output_test", allConditions);
}

/*
 function setHTML(ID, value) {
 if (document.getElementById(ID)) {
 document.getElementById(ID).innerHTML = value; //range
 } else {
 if (document.getElementById("test")) {
 // document.getElementById("test").innerHTML += "<br>wrong_setHTML:'" + ID + "' value:" + value; //range
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
 //document.getElementById("test").innerHTML += "<br>wrong_getHTML:'" + ID + "'"; //range
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
 xmlHttp.timeout = 1000;
 xmlHttp.ontimeout = function () {
 callback("no file");
 }
 xmlHttp.onreadystatechange = function () {

 if (xmlHttp.readyState === 4) {
 if (xmlHttp.status === 200) {
 callback(xmlHttp.responseText);
 } else {
 callback({"response": 0});
 }
 }

 }


 xmlHttp.send(null);
 }
 */

///////////////////////////////////////////////////////////
function getParameterByName(name, url) {
    if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, "\\$&");
    var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)"),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, " "));
}

var types = [];
var act = [];
var act_btn = new Array();

var selected_id_enabled = [];
var selected_typeCondition = new Array();
var selected_OptionAct = new Array();
var selected_OptionWhich = new Array();
var selected_typeActBtn = new Array();
selected_typeActChangeBtn = [];
var selected_typeAct = new Array();
var selected_bySignal = new Array();
var selected_timerField = new Array();
var selected_timerType = new Array();
var selected_by_pwm = [];
var selected_time = [];
var selected_date = [];
var selected_pwmTypeAct = [];
var typePinsPIN = [];
var typePinsVAL = [];
var typePinsIP = [];
//typePinsDuration
//typePinsTimeChoise
//typePinsrepeats
var typePinsDuration = [];
var typePinsTimeChoise = [];
var typePinsrepeats = [];
var typeDelay = [];

var typePins_minTemp = [];
var typePins_maxTemp = [];
var typePins_slaider = [];



var type = new Array();
var times = new Array();
var dates = new Array();
var act_ = new Array();
var actBtn = new Array();
var which = new Array();
var selected_typePins = new Array();
var signals = new Array();
var actBtn_ = new Array();
var signals_time = new Array();
var signals_time_convInt = new Array();

var nConditions;

var timeChoise = ["сек", "мин"];

function AddCondition(btnId, save_that) {
    nConditions = btnId;
    var result_which = "";
    var result_act = "";

    if (save_that) {
        SelectedSave(btnId);
    }

    //////////////////////////////
    var result = "";

    var add_option;
    var add_act;
    var selected = "";


    signals[0] = "положительный";
    signals[1] = "отрицательный";
    signals[2] = "больше:";
    signals[3] = "меньше:";
    signals[4] = "множитель:";

    types[0] = "нет";
    types[1] = "по достижению времени";
    types[2] = "по уровню";
    types[3] = "таймер";

    act[0] = "нет";
    act[1] = "переключить пин";
    act[2] = "переключить кнопку";
    act[3] = "перекл удаленную кнопку";
    act[4] = "отправить Email";
    act[5] = "переключить условие";
    act[6] = "перекл mqtt запрос";
    act[7] = "включить ленту 8211";
    act[8] = "WOL";
    act[9] = "настроить таймер";
    act[10] = "передвинуть слайдер";

    act_btn[0] = "нет";
    act_btn[1] = "1";
    act_btn[2] = "0";
    act_btn[3] = "шим";

    //signals_time[0] = "час";
    //signals_time[1] = "мин";
    signals_time[0] = "сек";


    var makeBtnid = btnId;

    makeAddbuttons(makeBtnid);

    document.getElementById("table").innerHTML += "<tbody id='body_table'><tbody>";
    document.getElementById("body_table").innerHTML += "<tr id='row_table" + btnId + "'>" +
        "<td id='enabled_table" + btnId + "'></td>" +
        "<td id='resultCondition_table" + btnId + "'></td>" +
        "<td id='resultCondition_choised_table" + btnId + "'></td>" +
        "<td id='resultWhich_table" + btnId + "'></td>" +
        "<td id='resultWhich_choised_table" + btnId + "'></td>" +
        "<td id='resultAct_table" + btnId + "'></td>" +
        "<td id='delete" + btnId + "'></td>" +
        "</tr>";

    setHTML("enabled_table" + btnId, "<input id='enabled" + btnId + "' type='checkbox' checked='true'>" + btnId);
    //document.getElementById("resultCondition_table" + btnId).innerHTML += "" + result + "";
    setHTML("resultCondition_table" + btnId, makeinOption(types, "typeCondition" + btnId, "typeConditionChange(" + btnId + ")"));
    document.getElementById("resultCondition_choised_table" + btnId).innerHTML += "<div id='resultCondition_choisedID" + btnId + "'></div>";
    //document.getElementById("resultWhich_table" + btnId).innerHTML += result_which;
    //document.getElementById("resultWhich_table" + btnId).innerHTML += makeinOption(act,"typeAct" + btnId,"typeActChange("+btnId+")");
    setHTML("resultWhich_table" + btnId, makeinOption(act, "typeAct" + btnId, "typeActChange(" + btnId + ")"));
    //document.getElementById("resultWhich_table" + btnId).appendChild(makeinOption_child(act,"typeAct" + btnId,"typeActChange("+btnId+")"));
    document.getElementById("resultWhich_choised_table" + btnId).innerHTML += "<div id='OptionWhich" + btnId + "'></div>";
    document.getElementById("resultAct_table" + btnId).innerHTML += "<div id='OptionAct" + btnId + "'></div>";
    setHTML("delete" + btnId, "<button class='form-control' onclick='deleteRow(" + btnId + ")'>X</button>");

    ///////////////////////достаем из переменной и вписываем
    SelectedLoad(btnId);

    ///////////////////////////////////////////////////////
}

function getRow() {
    var str = 'Give id="ab123" id="abc123" 100%!';
    var patt1 = new RegExp('id="\\w+', 'g'); //id="ab123"
    var result = str.match(patt1);
    var name;
    var id_number;
    for (i = 0; i < result.length; i++) {
        var patt2 = new RegExp('\\d+', 'g'); //123
        var patt3 = new RegExp('\\D+', 'g'); //adc
        id_number = result[i].match(patt2);
        name = result[i].match(patt3);
        name[0] = name[0].replace('id=', "").replace('"', "");
        //document.getElementById("demo").innerHTML += "<br>"+id_number[0];
        //document.getElementById("demo").innerHTML += "<br>"+name[0];
        //result[i]=
    }
    //document.getElementById("demo").innerHTML = result;
}

function createConditionVariable(btnId) {
    //SelectedLoad(btnId);
    //SelectedSave(btnId);
    for (var i = 0; i < btnId; i++) {
        AddCondition(i, false);
    }
    SelectedLoad(btnId);
    for (var i = 0; i < btnId; i++) {
        typeConditionChange(i); //выбираем "таймер" и автоматически создаются поля
    }
    SelectedLoad(btnId);
    for (var i = 0; i < btnId; i++) {
        SignalChange(i);
    }
    SelectedLoad(btnId);
    for (var i = 0; i < btnId; i++) {
        typeActChange(i);
    }
    SelectedLoad(btnId);
    for (var i = 0; i < btnId; i++) {
        typeActBtnChange(i);
    }
    SelectedLoad(btnId);
    for (var i = 0; i < btnId; i++) {
        typeActChangeBtn(i);
    }
    SelectedLoad(btnId);
}

function deleteRow(btnId) {
    SelectedSave(btnId);

    setHTML("table", "");
    selected_id_enabled.splice(btnId);


    selected_timerField.splice(btnId);
    selected_by_pwm.splice(btnId);
    selected_time.splice(btnId);
    selected_date.splice(btnId);
    selected_timerType.splice(btnId);
    selected_bySignal.splice(btnId);
    selected_typeCondition.splice(btnId);
    selected_typeActBtn.splice(btnId);
    type.splice(btnId);
    selected_typeAct.splice(btnId);
    selected_typePins.splice(btnId);
    act_.splice(btnId);
    selected_pwmTypeAct.splice(btnId);
    actBtn_.splice(btnId);
    signals_time_convInt.splice(btnId);

    nConditions = btnId;
    createConditionVariable(btnId);
    makeAddbuttons(btnId - 1);

    SelectedLoad(btnId);
}

function deleteRow1(btnId) {

    var data = document.getElementsByTagName('table')[0].innerHTML;
    var new_string;
    //data = data.replace(new RegExp('row_table' + btnId, 'g'), 'row_tableDel');

    var str = data;
    var patt1 = new RegExp('id="\\w+', 'g'); //id="ab123"
    var result = str.match(patt1);
    var name;
    var id_number;

    for (i = 0; i < result.length; i++) {
        var patt2 = new RegExp('\\d+', 'g'); //123
        var patt3 = new RegExp('\\D+', 'g'); //adc
        id_number = result[i].match(patt2);
        name = result[i].match(patt3);
        name[0] = name[0].replace('id=', "").replace('"', "");
        //document.getElementById("demo").innerHTML += "<br>"+id_number[0];
        //document.getElementById("demo").innerHTML += "<br>"+name[0];
        //result[i]=
    }


    for (i = btnId; i < 10; i++) {
        data = data.replace(new RegExp('row_table' + i, 'g'), 'row_table' + (i - 1));


        /*    data = data.replace(new RegExp('enabled' + i, 'g'), 'enabled' + (i - 1));
         data = data.replace(new RegExp('timerField' + i, 'g'), 'timerField' + (i - 1));
         data = data.replace(new RegExp('by_pwm' + i, 'g'), 'by_pwm' + (i - 1));
         data = data.replace(new RegExp('times' + i, 'g'), 'times' + (i - 1));
         data = data.replace(new RegExp('dates' + i, 'g'), 'dates' + (i - 1));
         data = data.replace(new RegExp('timerType' + i, 'g'), 'bySignal' + (i - 1));
         data = data.replace(new RegExp('typeCondition' + i, 'g'), 'typeCondition' + (i - 1));
         data = data.replace(new RegExp('typeActBtn' + i, 'g'), 'typeActBtn' + (i - 1));
         data = data.replace(new RegExp('typeAct' + i, 'g'), 'typeAct' + (i - 1));
         data = data.replace(new RegExp('typePins' + i, 'g'), 'typePins' + (i - 1));
         data = data.replace(new RegExp('pwmTypeAct' + i, 'g'), 'pwmTypeAct' + (i - 1));
         */
        //data = new_string;
    }
    document.getElementsByTagName('table')[0].innerHTML = data;
    setHTML("row_tableDel", "");
    var data = document.getElementsByTagName('table')[0].innerHTML;
    data = data.replace(new RegExp('<tr id="row_tableDel"></tr>', 'g'), '');
    document.getElementsByTagName('table')[0].innerHTML = data;

    SelectedLoad(btnId);

    /*
     selected_id_enabled.splice(i, 1);
     selected_timerField.splice(i, 1);
     selected_timerType.splice(i, 1);
     selected_bySignal.splice(i, 1);
     selected_typeCondition.splice(i, 1);
     selected_typeAct.splice(i, 1);
     selected_typePins.splice(i, 1);
     selected_typeActBtn.splice(i, 1);
     selected_by_pwm.splice(i, 1);
     selected_time.splice(i, 1);
     selected_date.splice(i, 1);
     selected_pwmTypeAct.splice(i, 1);
     */
    //btnId--;
    //makeAddbuttons(btnId);


}

function makeAddbuttons(btnId) {
    if (btnId < Naumber_in_condition) {
        btnId++;
        if (btnId === undefined) {
            btnId = 0;
        }
        setHTML("firstButton", "<input type='submit' class='btn btn-lg btn-primary btn-block' value='добавить условие:" + (btnId) + "' onclick='AddCondition(" + (btnId) + ",true);' />");
        //setHTML("firstButton", getHTML("firstButton") + "<input type='submit' class='btn btn-lg btn-primary btn-block' value='удалить условие:" + (btnId - 1) + "' onclick='deleteRow(" + (btnId - 1) + ");' />");
    } else {
        btnId++;
        setHTML("firstButton", "<input type='submit' class='btn btn-lg btn-primary btn-block' value='Достигли ограничения:");
        //setHTML("firstButton", getHTML("firstButton") + "<input type='submit' class='btn btn-lg btn-primary btn-block' value='удалить условие:" + (btnId - 1) + "' onclick='deleteRow(" + (btnId - 1) + ");' />");

    }
    setHTML("firstButton", getHTML("firstButton") + "<input type='submit' class='btn btn-lg btn-primary btn-block' value='сохранить' onclick='save(" + (btnId - 1) + ")' />");
    for (i1 = 0; i1 < btnId; i1++) {
        //typeConditionChange(i1);
        typeActChange(i1);
    }
}

function convertTimetoHMS(times) {

    var seconds = times;
    // multiply by 1000 because Date() requires miliseconds
    var date = new Date(seconds * 1000);
    var hh = date.getUTCHours();
    var mm = date.getUTCMinutes();
    var ss = date.getSeconds();

    if (hh < 10) {
        hh = "0" + hh;
    }
    if (mm < 10) {
        mm = "0" + mm;
    }
    if (ss < 10) {
        ss = "0" + ss;
    }
    // This formats your string to HH:MM:SS
    return t = hh + ":" + mm + ":" + ss;
}

function convertTimetoHM(times) {

    var minutes = times;
    // multiply by 1000 because Date() requires miliseconds
    var date = new Date(minutes * 1000 * 60);
    var hh = date.getUTCHours();
    var mm = date.getUTCMinutes();
    var ss = date.getSeconds();

    if (hh < 10) {
        hh = "0" + hh;
    }
    if (mm < 10) {
        mm = "0" + mm;
    }
    if (ss < 10) {
        ss = "0" + ss;
    }
    // This formats your string to HH:MM:SS
    return t = hh + ":" + mm + ":" + ss;
}

function getCondBack(act, actBtn2, i) {
    var val = actBtn2;
    switch (act) {
        case 2:
            val = jsonPinSetup.descr[actBtn2];
            break;
        case 5:
            arr = actBtn2.split(' ');
            if (arr.length > 0) {

                var i2 = arr[0];
                var i1 = arr[1];
                //document.getElementById("output").appendChild(alert_message(jsonCondtion[i2]));
                if ((jsonCondtion[i2])) {

                    if ((jsonCondtion[i2].tID[i1] !== undefined) && ((jsonPinSetup.descr[i2] !== undefined))) {

                        if ((i2 === "-1") || (i1 === "-1")) {
                            return 0;
                        }
                        //if ((typeof jsonPinSetup.descr[i2]=="undefined")||(typeof jsonCondtion[i2].tID[i1]=="undefined")){return 0;}
                        val = jsonPinSetup.descr[i2] + ":" + jsonCondtion[i2].tID[i1];//возможна ошибка если построение typeActChange будет другое
                    }
                }
                /*
                 if ((i2==="-1")||(i1==="-1")){return 0;}
                 //if ((typeof jsonPinSetup.descr[i2]=="undefined")||(typeof jsonCondtion[i2].tID[i1]=="undefined")){return 0;}
                 val=jsonPinSetup.descr[i2] + ":" + jsonCondtion[i2].tID[i1];//возможна ошибка если построение typeActChange будет другое
                 */

            } else {
                return 0;
            }
            break;
        case 9:
            arr = actBtn2.split(' ');
            //alert(arr);
            if (arr.length > 0) {
                typePinsDuration[i] = arr[0];
                //typePinsTimeChoise=timeChoise[arr[1]];
                //typePinsTimeChoise=;
                typePinsTimeChoise[i] = timeChoise[arr[1]];
                //setVal("typePinsTimeChoise" + i, typePinsTimeChoise);
                //setVal("typePinsTimeChoise0","сек");
                //alert(document.getElementById("typePinsTimeChoise0"));
                //setVal("typePinsTimeChoise" + i,"мин");
                //alert(timeChoise[arr[1]]);
                //setVal("typePinsTimeChoise" + i,timeChoise[arr[1]]);
                typePinsrepeats[i] = arr[2];
                typeDelay[i] = arr[3];
                // setHTML("NextRepeat"+i,readTextFile());
                var id = document.getElementById("NumberWidget").selectedIndex;


                readTextFile("function?json={\"NextRepeat\":1,\"NextRepeatCondition\":" + id + ",\"NextRepeatNumber\":" + i + "}", function (callback) {
                    var data = JSON.parse(callback);
                    document.getElementById("output").appendChild(alert_message(JSON.stringify(callback)));
                    var time = data.times;
                    // var time=28923;
                    var hour = Math.trunc(time / 3600);
                    var minute = Math.trunc(time % 3600 / 60);
                    var second = Math.trunc(time % 60);
                    time = hour + ":" + minute + ":" + second;
                    setHTML("NextRepeat" + data.Number, time);
                    //setHTML("NextRepeat0", time);
                })
            }
            break;
        case 10:

                 typePins_minTemp[i] = 0;
                 typePins_maxTemp[i] = 0;
                 typePins_slaider[i] = 0;

            break;
        default:
            return val;
    }
    return val;
}

function createCondition(data) {
    setHTML("table", "");
    makeAddbuttons(-1);
    deleteRow(0);
    
    btnId = data.tID.length;
    
    for (var i = 0; i < data.tID.length; i++) {
        AddCondition(i, true);
        
    }
    nConditions = data.tID.length;
    
    //setHTML("loadedCondition",getHTML("loadedCondition")+"<br>addConditionOk!");
    for (var i = 0; i < data.tID.length; i++) {
        //setHTML("test",getHTML("test") +"<br>timer:"+data.actBtn[i]);
        selected_id_enabled[i] = data.En ? data.En[i] : 0;
        selected_typeCondition[i] = data.type ? types[data.type[i]] : 0;
        selected_timerField[i] = data.times ? data.times[i] : 0;
        selected_timerType[i] = data.timerType ? signals_time[data.timerType[i]] : 0;
        selected_typeAct[i] = data.act ? act[data.act[i]] : 0;
        selected_typeActBtn[i] = data.actOn ? act_btn[data.actOn[i]] : 0;
        selected_time[i] = data.times ? convertTimetoHMS(data.times[i]) : 0;
        selected_date[i] = data.dates ?data.dates[i] : 0;
        //selected_typePins[i] = jsonPinSetup.descr[data.actBtn[i]];
        selected_typePins[i] = data.actBtn && data.act ? getCondBack(data.act[i], data.actBtn[i], i) : 0;// data.actBtn[i];- default
        selected_bySignal[i] = data.bySignal ? signals[data.bySignal[i]] : 0;
        selected_by_pwm[i] = data.bySignalPWM ? data.bySignalPWM[i] : 0;
        selected_pwmTypeAct[i] = data.pwmTypeAct[i];


    }
    //setHTML("loadedCondition",getHTML("loadedCondition")+"<br>  for (var i = 0; i < data.tID.length; i++) { Ok!");
    SelectedLoad(btnId);
    //setHTML("loadedCondition",getHTML("loadedCondition")+"<br> SelectedLoad Ok!");
    for (var i = 0; i < data.tID.length; i++) {
        typeConditionChange(i); //выбираем "таймер" и автоматически создаются поля
    }
    SelectedLoad(btnId);
    for (var i = 0; i < data.tID.length; i++) {
        SignalChange(i);
    }
    SelectedLoad(btnId);
    for (var i = 0; i < data.tID.length; i++) {
        typeActChange(i);
        getCondBack(data.act[i], data.actBtn[i], i);
    }
    SelectedLoad(btnId);
    for (var i = 0; i < data.tID.length; i++) {

        typeActBtnChange(i);
    }
    SelectedLoad(btnId);
    for (var i = 0; i < data.tID.length; i++) {
        typeActChangeBtn(i);
    }
    SelectedLoad(btnId);

}

function convertTime(selected_time) {
    var pctime = [];
    for (i = 0; i < selected_time.length; i++) {
        var a = [];
        var seconds;
        try {
            a = selected_time[i].split(':'); // split it at the colons
            seconds = (+a[0]) * 60 * 60 + (+a[1]);
        } catch (e) {
            seconds = -1;
        }
        pctime[i] = seconds;
    }
    return pctime;
}

function convertTime_min(selected_time) {
    var pctime = [];
    for (i = 0; i < selected_time.length; i++) {
        var a = [];
        var min;
        try {
            a = selected_time[i].split(':'); // split it at the colons
            min = (+a[0]) * 60 + (+a[1]);
        } catch (e) {
            min = -1;
        }
        pctime[i] = min;
    }
    return pctime;
}
// function getTimerField(type, act, i) {
//     var return_value;
//     switch (type) {
//         case 3://таймер
//             return_value = parseInt(convertTime(selected_time)[i] === -1 ? selected_timerField[i] : convertTime(selected_time)[i]);

//             break;
//         case 2:
//             switch (act) {
//                 case 10://передвинуть слайдер
//                     return_value = getVal("typePins_minTemp" + i);
//                     break;
//             }
//             break;

//     }

//     return return_value;
// }
// function getTimerType(type, act, i) {
//     var return_value;
//     switch (type) {
//         case 3://таймер
        
//             return_value = signals_time.indexOf(selected_timerType[i]) === -1 ? 255 : signals_time.indexOf(selected_timerType[i]);
//             break;
//         case 2:
//             switch (act) {
//                 case 10://передвинуть слайдер
//                     return_value = getVal("typePins_maxTemp" + i);
//                     break;
//             }

            
//     }
//     return return_value;
// }
function save(btnId) {
    SelectedSave(btnId);
    var jsonStr2 = {};
    var BtnIDArray = new Array();
    var bySignal = [];
    var selected_id_enabled_bool = [];

    var timer_field_ = [];
    for (var i = 0; i < (btnId + 1); i++) { //сперва сохраняем весь выбор в переменную\
        selected_id_enabled_bool[i] = Number(getBool(selected_id_enabled[i]));
        BtnIDArray[i] = i;
        type[i] = types.indexOf(selected_typeCondition[i]);
        act_[i] = act.indexOf(selected_typeAct[i]);
        actBtn_[i] = act_btn.indexOf(selected_typeActBtn[i]);
        signals_time_convInt[i] = signals_time.indexOf(selected_timerType[i]) === -1 ? 255 : signals_time.indexOf(selected_timerType[i]);
        //signals_time_convInt[i] = getTimerType(type[i], act_[i], i);
        //if (selected_bySignal[i]!=-1)
        bySignal[i] = signals.indexOf(selected_bySignal[i]);
        //document.getElementById("test").innerHTML = "123"+act_;
        selected_typePins[i] = getPinsIfCond(act_[i], selected_typePins[i], i);
        timer_field_[i] = parseInt(convertTime(selected_time)[i] === -1 ? selected_timerField[i] : convertTime(selected_time)[i]);
    }
    /*
     for (i = 0; i < jsonPinSetup.id.length; i++) {
     if (jsonCondtion[i]) {
     for (i1 = 0; i1 < jsonCondtion[i].tID.length; i1++) {
     add_pins += "<option " + selected + " >" + jsonPinSetup.descr[i] + ":" + jsonCondtion[i].tID[i1] + " " + types[jsonCondtion[i].type[i1]]+ "</option>";
     }
     }
     }
     */
    //var id = 0;
    if (document.getElementById("NumberWidget")) {
        id = document.getElementById("NumberWidget").selectedIndex; //устанавливаем из страницы
    } else {
        id = 0;
    }
    var time = selected_time;
    //setHTML("output_test", time);
    //setHTML("output_test",convertTimetoHMS(15457));
    jsonStr2["tID"] = BtnIDArray;
    jsonStr2["En"] = selected_id_enabled_bool;
    jsonStr2["times"] = timer_field_;
    //jsonStr2.times.splice(btnId+1);
    //jsonStr2["dates"] = selected_date;
    jsonStr2["bySignal"] = bySignal;
    jsonStr2["bySignalPWM"] = selected_by_pwm;
    jsonStr2["type"] = type;

    if (selected_timerField != "") {
        // jsonStr2["timer"] = selected_timerField;
        jsonStr2["timerType"] = signals_time_convInt;
    }

    jsonStr2["act"] = act_;
    jsonStr2["actBtn"] = selected_typePins;
    jsonStr2["actOn"] = actBtn_;
    jsonStr2["pwmTypeAct"] = selected_pwmTypeAct;


    jsonStr2["ID"] = id;
    jsonStr2["Numbers"] = btnId + 1;
    json = JSON.stringify(jsonStr2);

    //document.getElementById("testJSON").innerHTML = JSON.stringify(jsonStr2);
    var jsonPretty = JSON.stringify(JSON.parse(json), null, 2);
    //document.getElementById("testJSON").innerHTML = jsonPretty;

    var json_upload = "json_name=" + jsonPretty;
    //json_upload.append("Number", btnId);
    setHTML("output", json);
    saveData("Condition" + id + ".txt", jsonStr2, false);
    jsonCondtion[id] = JSON.parse(json);
    /*
     var xmlHttp = createXmlHttpObject();
     xmlHttp.open("POST", '/condition_setup', true);
     xmlHttp.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
 
     xmlHttp.send(json_upload);
 
     xmlHttp.onloadend = function () {
     setHTML("output", json);
     };
     jsonCondtion[id] = JSON.parse(json);
     xmlHttp.onreadystatechange = function () {
     if (xmlHttp.readyState === 4 && xmlHttp.status == "200") {
     //callback(xmlHttp.responseText);
     alert(xmlHttp.responseText);
     }
     }
     */
}

function getBool(val) {
    return (!!JSON.parse(String(val).toLowerCase()));
}

/*
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
 */

/*
здесь мы получаем от форм
*/
function getPinsIfCond(act2, def_pin, i) {
    var pin = def_pin;
    switch (act2) {//например переключить кнопку, переключить пин переключить удаленную кнопку

        case 2://переключить кнопку

            /*
                        var thatIndex = 0;
                        if (document.getElementById("typePins" + i).selectedIndex) {
                            thatIndex = document.getElementById("typePins" + i).selectedIndex;
                        }
                        pin = thatIndex;
            */
            pin = "";
            // if (ButtonAddFieldClick_id > 0) {
            //     for (var idCl = 0; idCl < ButtonAddFieldClick_id; idCl++) {
            //         pin += jsonPinSetup.descr.indexOf(getVal("ButtonAddField" + i + idCl));
            //         //alert("ButtonAddField" + i + idCl);
            //         //if((idCl!==0)&&(idCl!==ButtonAddFieldClick_id)){}
            //         pin=((idCl !== ButtonAddFieldClick_id-1)) ? pin += " " : pin;
            //     }
            // }
            pin = jsonPinSetup.descr.indexOf(document.getElementById("typePins" + i).value);
            break;

        case 5:
            var cond = -1;
            var NumbCond = -1;
            var FindIndex = 0;
            var thatIndex = 0;
            if (document.getElementById("typePins" + i).selectedIndex) {
                thatIndex = document.getElementById("typePins" + i).selectedIndex;
            }

            for (i2 = 0; i2 < jsonPinSetup.numberChosed; i2++) {
                if (testJson(jsonCondtion[i2])) {
                    for (i1 = 0; i1 < jsonCondtion[i2].tID.length; i1++) {
                        if (FindIndex === thatIndex) {
                            // if (find == (jsonPinSetup.descr[i2] + ":" + jsonCondtion[i2].tID[i1] + " " + types[jsonCondtion[i2].type[i1]])) {
                            cond = i2;
                            NumbCond = i1;
                        }
                        FindIndex++;
                    }
                }
            }
            pin = cond + ":" + NumbCond;
            arrayCond = getVal("typePins" + i).split(':');
            pin = jsonPinSetup.descr.indexOf(arrayCond[0]) + " " + arrayCond[1];

            break;
        case 6:
            //var arrayCond = getVal("typePins" + i).split(':');

            //pin=arrayCond[0];
            break;
        case 9:
            //pin=
            var Duration = getVal("typePinsDuration" + i) !== "" ? getVal("typePinsDuration" + i) : 0;
            var TimeChoise = document.getElementById("typePinsTimeChoise" + i).selectedIndex;
            var Pinsrepeats = getVal("typePinsrepeats" + i) !== "" ? getVal("typePinsrepeats" + i) : 0;
            var pinDelay = Number(getVal("typeDelay" + i));
            // getVal("typePinsDuration" + i)+":"+
            // getVal("typePinsTimeChoise" + i)+":"+
            //getVal("typePinsrepeats" + i);
            pin = Duration + " " + TimeChoise + " " + Pinsrepeats + " " + pinDelay;
            break;
        case 10:

            //jsonPinSetup.descr.indexOf();
            var typePins_minTemp_ = getVal("typePins_minTemp" + i) !== "" ? getVal("typePins_minTemp" + i) : 0;
            var typePins_maxTemp_ = getVal("typePins_maxTemp" + i) !== "" ? getVal("typePins_maxTemp" + i) : 0;
            var typePins_slaider_ = getVal("typePins_slaider" + i) !== "" ? getVal("typePins_slaider" + i) : 0;

            typePins_slaider_ = jsonPinSetup.descr.indexOf(document.getElementById("typePins_slaider" + i).value);
            pin = typePins_minTemp_ + " " + typePins_maxTemp_ + " " + typePins_slaider_;
            //переделаю обратно
            pin = jsonPinSetup.descr.indexOf(document.getElementById("typePins_slaider" + i).value);

            break;
    }
    return pin;
}

function typeActBtnChange(btnId) {

    var selected_pin = getVal("typePins" + btnId); //document.getElementById("typePins" + btnId).value;
    var selected_do = getVal("typeAct" + btnId);
    var result_act_btn = "";
    var selected = "";
    var setHtml = "";
    switch (selected_do) {
        // case act[2]: //переключить кнопку

        case act[3]: //нажать удаленную кнопку
        case act[6]: //нажать mqtt кнопку
        case act[4]: //отправить Email
        case act[7]: //включить ленту
        case act[8]: //WOL
            result_act_btn = "";
            break;
        case act[5]: //условие
            setHtml = makeinOption(act_btn, "typeActBtn" + btnId, "typeActBtn(" + btnId + ")") + "<div id='typeActIfPWM" + btnId + "'></div>";
            break;
        case act[0]:
            result_act_btn = "";
            break;
        //case act[9]:
        // var element2 =  document.getElementById("OptionAct" + btnId);
        //element2.appendChild(makeinOption_child(act_btn, "typeActBtn" + btnId, false));

        //setHtml = makeinOption(act_btn, "typeActBtn" + btnId, "typeActBtn(" + btnId + ")") + "<div id='typeActIfPWM" + btnId + "'></div>";
        //setHtml += makeinOption(act_btn, "typeActBtn" + btnId, "typeActBtn(" + btnId + ")") + "<div id='typeActIfPWM" + btnId + "'></div>";
        // break;
        default:

            setHtml = makeinOption(act_btn, "typeActBtn" + btnId, "typeActChangeBtn(" + btnId + ")") + "<div id='typeActIfPWM" + btnId + "'></div>";

            break;
    }
    setHTML("OptionAct" + btnId, setHtml);
}

function typeActChangeBtn(btnId) {
    var result = "";
    var choised;
    //choised = document.getElementById("typeActBtn" + btnId).value;
    choised = getVal("typeActBtn" + btnId);
    switch (choised) {
        case act_btn[3]: //PWM
            result = "<input class='form-control' id='pwmTypeAct" + btnId + "' type='text' value='' size='1'>";
            break;

    }
    setHTML("typeActIfPWM" + btnId, result);
    //document.getElementById("typeActIfPWM" + btnId).innerHTML = result;
}

function typeActChange(btnId) {

    //document.getElementById("OptionWhich" + btnId).innerHTML = "";
    setHTML("OptionWhich" + btnId, "");

    var pins = new Array();
    var inputPin = new Array();
    var result_pins = "";
    var add_pins = "";
    var selected = "";

    var typeActChoised = getVal("typeAct" + btnId); // document.getElementById().value;
    switch (typeActChoised) {
        case act[1]: //установить пин
            for (i = 0; i <= 17; i++) {

                pins[i] = "" + i + "";
            }
            inputPin[-1] = "no";
            inputPin[0] = 0;
            //inputPin[2] = 1;//txd
            inputPin[2] = 2;
            //inputPin[4] = 3;//rxd
            inputPin[4] = 4;
            inputPin[5] = 5;
            //inputPin[6] = 6;//перезагрузка
            //inputPin[7] = 7;//перезагрузка
            //inputPin[8] = 8;//перезагрузка
            //inputPin[9] = 9;//перезагрузка
            inputPin[10] = 10; //d3
            //inputPin[11] = 11;//перезагрузка
            inputPin[12] = 12;
            inputPin[13] = 13;
            inputPin[14] = 14;
            inputPin[15] = 15;
            inputPin[16] = 16; //xpd
            inputPin[17] = 17; //adc

            //document.getElementById("OptionWhich" + btnId).innerHTML += pins;
            for (i = 0; i < inputPin.length; i++) {
                add_pins += "<option " + selected + " >" + inputPin[i] + "</option>";
            }
            //document.getElementById("OptionWhich" + btnId).innerHTML += pins+add_pins;
            result_pins += "<select class='form-control' id='typePins" + btnId + "' onchange='typeActBtnChange(" + btnId + ");' >" + add_pins + "</select>";
            setHTML("OptionWhich" + btnId, getHTML("OptionWhich" + btnId) + result_pins);
            break;
        case act[2]: //нажать кнопку
            if (jsonPinSetup) {
                for (i = 0; i < jsonPinSetup.numberChosed; i++) {
                    add_pins += "<option " + selected + " >" + jsonPinSetup.descr[i] + "</option>";
                }
            }
            result_pins += "<select class='form-control' id='typePins" + btnId + "' onchange='typeActBtnChange(" + btnId + ");' >" + add_pins + "</select>";
            setHTML("OptionWhich" + btnId, getHTML("OptionWhich" + btnId) + result_pins);

            break;
        case act[3]: //нажать удаленную кнопку
            result_pins += "<input class='form-control' id='typePins" + btnId + "' type='text' placeholder='192.168.1.108/aRest?Json={pin:1,val:100}' title='192.168.1.108/aRest?Json={pin:1,val:100} \n 192.168.1.108- адрес устройства \n pin - номер ножки,\n val - ее значение' value='192.168.1.108/aRest?Json={pin:1,val:100}' size='100'>";
            //result_pins += "<div class='row'><div class='col-md-6'>IP:<input class='form-control' id='typePinsIP" + btnId + "' type='text' placeholder='192.168.1.108/aRest?Json={pin:1,val:100}' title='192.168.1.108/aRest?Json={pin:1,val:100} \n 192.168.1.108- адрес устройства \n pin - номер ножки,\n val - ее значение' value='' size='10'></div>";
            //result_pins += "<div class='col-md-6'>PIN:<input class='form-control' id='typePinsPIN" + btnId + "' type='text' value='' size='2'></div>";
            //result_pins += "<div class='col-md-6'>VAL:<input class='form-control' id='typePinsVAL" + btnId + "' type='text' value='' size='2'></div></div>";
            setHTML("OptionWhich" + btnId, getHTML("OptionWhich" + btnId) + result_pins);//
            break;
        case act[4]: //отправить Email
        case act[8]://WOL
            result_pins += "<input class='form-control' id='typePins" + btnId + "' type='text' placeholder='ваш текст' title='вписать текст сообщения' value='' size='100'>";
            setHTML("OptionWhich" + btnId, getHTML("OptionWhich" + btnId) + result_pins);
            break;
        case act[5]: //включить или отключить условие

            add_pins = "";//typePins
            for (i1 = 0; i1 < selected_typeCondition.length; i1++) {
                //add_pins += "<option " + selected + " >" + selected_typeCondition[i1]+ "</option>";
            }
            for (i = 0; i < jsonPinSetup.numberChosed; i++) {
                //document.getElementById("output").appendChild(alert_message(JSON.stringify(jsonCondtion[i])));

                //if ((testJson(jsonCondtion[i].tID)) && (i !== document.getElementById("NumberWidget").selectedIndex)) {
                if ((jsonCondtion[i].tID !== undefined) && (i !== document.getElementById("NumberWidget").selectedIndex)) {
                    //document.getElementById("output").appendChild(alert_message());
                    //
                    for (i1 = 0; i1 < jsonCondtion[i].tID.length; i1++) {// записанные условия
                        add_pins += "<option " + selected + " >" + jsonPinSetup.descr[i] + ":" + jsonCondtion[i].tID[i1] + "</option>";
                    }
                }
                if ((i === document.getElementById("NumberWidget").selectedIndex)) {//текущие условия
                    for (i1 = 0; i1 < selected_id_enabled.length; i1++) {
                        add_pins += "<option " + selected + " >" + jsonPinSetup.descr[document.getElementById("NumberWidget").selectedIndex] + ":" + i1 + "</option>";
                    }
                }

            }

            result_pins += "<select class='form-control' id='typePins" + btnId + "' onchange='typeActBtnChange(" + btnId + ");' >" + add_pins + "</select>";
            setHTML("OptionWhich" + btnId, getHTML("OptionWhich" + btnId) + result_pins);
            break;
        case act[6]: //нажать mqtt кнопку
            result_pins += "<input class='form-control' id='typePins" + btnId + "' type='text' title='{\"Topic\":\"dev01/button/id\",\"msg\":\"1\"} \n dev01/button/id- топик \n 1 - сообщение' value='{\"Topic\":\"-=topic=-\",\"msg\":\"-=messgage=-\"}' size='100'>";
            setHTML("OptionWhich" + btnId, getHTML("OptionWhich" + btnId) + result_pins);//
            break;
        case act[7]://ws8211
            //makeinOption_child;
            var array = [0, 1, 2, 3, 4, 5];
            var element = document.getElementById("OptionWhich" + btnId);
            element.appendChild(makeinOption_child(array, "typePins" + btnId, false));
            break;
        case act[9]://ws8211
            //makeinOption_child;
            var array = [];
            var i = 0;
            while (i < 254) {
                array[i] = i;
                i++;
            }
            var element = document.getElementById("OptionWhich" + btnId);

            //var h = document.createElement("p")                // Create a <h1> element
            var t = document.createTextNode("продолжительность");     // Create a text node
            element.appendChild(t);
            var input = document.createElement("INPUT");

            input.setAttribute("type", "number");
            input.className = "form-control";
            input.setAttribute("id", "typePinsDuration" + btnId);
            element.appendChild(input);
            //
            element.appendChild(makeinOption_child(timeChoise, "typePinsTimeChoise" + btnId, false));
            var that = document.getElementById("typePinsTimeChoise" + btnId);
            //
            t = document.createTextNode("повторы");
            element.appendChild(t);
            //
            input = document.createElement("INPUT");
            input.className = "form-control";
            input.setAttribute("type", "number");
            input.setAttribute("id", "typePinsrepeats" + btnId);
            element.appendChild(input);
            //
            t = document.createTextNode("между повторами (мин)");
            element.appendChild(t);
            //
            input = document.createElement("INPUT");
            input.className = "form-control";
            input.setAttribute("type", "number");
            input.setAttribute("id", "typeDelay" + btnId);
            element.appendChild(input);
            //
            t = document.createTextNode("следующий повтор:");
            element.appendChild(t);
            //
            t = document.createElement("p");
            t.innerHTML = "wait";
            t.setAttribute("id", "NextRepeat" + btnId);
            element.appendChild(t);
            break;
        case act[10]: //передвинуть слайдер
            /*
                    71 - 1024
                        53 - 0
            
                        71 - 53 = 18
            
                        например температура 71
                        тогда 1024/71 = 14
                        и 14 * 71 = 1024
                        если температура 53, то 53 * 14 = 742, а должно быть 0
            
                        значит если 1024/(71 - 53) = 56
                        не подходит
            
                        значит (53 - 53) * 14  = 0
                        (71 - 53) * 14 = 252, а должно быть 1024
            
                        1024/252 = 4
            
                        4 * 14 = 56
            
                        (71 - 53) * (1024 / (71-53)) = 1008
                        (53 - 53) * 56 = 1008
            
                        (temp - Temp_min) * (1024 / (tmax-tmin)) = 
                        */
            //текст
            var element = document.getElementById("OptionWhich" + btnId);
            var t = document.createTextNode("мин темп:");     // Create a text node
            element.appendChild(t);
            //ввод минмальной температуры
            var input = document.createElement("INPUT");
            input.setAttribute("type", "number");
            input.className = "form-control";
            input.setAttribute("id", "typePins_minTemp" + btnId);
            element.appendChild(input);
            //текст
            var element = document.getElementById("OptionWhich" + btnId);
            var t = document.createTextNode("макс темп:");     // Create a text node
            element.appendChild(t);
            //ввод максимальной температуры
            var input = document.createElement("INPUT");
            input.setAttribute("type", "number");
            input.className = "form-control";
            input.setAttribute("id", "typePins_maxTemp" + btnId);
            element.appendChild(input);
            //текст
            var t = document.createTextNode("слайдер:");     // Create a text node
            element.appendChild(t);
            //
            //
            filtered_widget = [];
            if (jsonPinSetup) {
                i1 = 0;
                for (i = 0; i < jsonPinSetup.numberChosed; i++) {
                    if (jsonPinSetup.widget[i] === 3) {
                        //add_pins += "<option " + selected + " >" + jsonPinSetup.descr[i] + "</option>";
                        filtered_widget[i1] = jsonPinSetup.descr[i];
                        i1++;
                    }
                }
            }
            element.appendChild(makeinOption_child(filtered_widget, "typePins_slaider" + btnId, false));


            //
            // if (jsonPinSetup) {
            //     for (i = 0; i < jsonPinSetup.numberChosed; i++) {
            //         if (jsonPinSetup.widget[i] === 3) {
            //             add_pins += "<option " + selected + " >" + jsonPinSetup.descr[i] + "</option>";
            //         }
            //     }
            // }
            // result_pins += "<select class='form-control' id='typePins" + btnId + "' onchange='typeActBtnChange(" + btnId + ");' >" + add_pins + "</select>";
            // setHTML("OptionWhich" + btnId, getHTML("OptionWhich" + btnId) + result_pins);

            break;
    }
    //typeActChange(btnId)
    typeActBtnChange(btnId);
    //typeActChangeBtn(btnId);


}

var ButtonAddFieldClick_id = 0;

function ButtonAddFieldClick(id_btn) {

    var element = document.getElementById("inputSignalChange" + id_btn);
    element.appendChild(makeinOption_child(jsonPinSetup.descr, "ButtonAddField" + id_btn + ButtonAddFieldClick_id, false));
    ButtonAddFieldClick_id++;
}

function typeConditionChange(btnId) {
    document.getElementById("resultCondition_choisedID" + btnId).innerHTML = "";

    var result_srting = "";
    var typeConditionChoised = document.getElementById("typeCondition" + btnId).value;

    switch (typeConditionChoised) {
        case types[1]: // по времени
            if (document.getElementById("times" + (btnId - 1))) {
                var prevTimeSet = document.getElementById("times" + (btnId - 1)).value;
                result_srting += "<input class='form-control' id='times" + btnId + "' type='time' value='" + prevTimeSet + "' size='1'>";
            } else {
                result_srting += "<input class='form-control' id='times" + btnId + "' type='time' value='08:00:00' size='1'>";
            }
            //result_srting += "<input class='form-control' id='dates" + btnId + "' type='date' value='00' size='1'>";
            break;
        case types[2]: //по уровню

            var add_signals = "";
            var selected = "";

            for (var i = 0; i < signals.length; i++) {
                add_signals += "<option >" + signals[i] + "</option>";
            }

            result_srting += "<select class='form-control' id='bySignal" + btnId + "' onchange='SignalChange(" + btnId + ");' >" + add_signals + "</select>";
            result_srting += "<div id='inputSignalChange" + btnId + "'></div>";
            result_srting += "<button class='form-control' id='inputSignalChange" + btnId + "' onclick='ButtonAddFieldClick(" + btnId + ")'>add</button>";
            break;
        case types[3]: //таймер

            var add_signals = "";
            var selected = "";

            for (var i = 0; i < signals_time.length; i++) {
                add_signals += "<option >" + signals_time[i] + "</option>";
            }
            result_srting += "<input class='form-control' id='timerField" + btnId + "' type='input' size='1'>";
            result_srting += "<select class='form-control' id='timerType" + btnId + "' onchange='timerOnChange(" + btnId + ");' >" + add_signals + "</select>";
            result_srting += "<div id='inputSignalChange" + btnId + "'></div>";
            getTimeOfNextRepeat(btnId);
            break;
        case types[4]: //таймер по времени
            var timeset = '08:00:00';
            if (getVal("times" + (btnId - 1)) !== -1) {
                timeset = getVal("times" + (btnId - 1));
            }
            result_srting += "<input class='form-control' id='times" + btnId + "' type='time' value='" + timeset + "' size='1'>";
            break;
    }

    //document.getElementById("resultCondition_choisedID" + btnId).innerHTML = result_srting;
    setHTML("resultCondition_choisedID" + btnId, result_srting)
}

function getTimeOfNextRepeat(btnId) {
    var element = document.getElementById("resultCondition_choised_table" + btnId);
    var t = document.createElement("p");
    t.innerHTML = "wait";
    t.setAttribute("id", "NextRepeat" + btnId);
    element.appendChild(t);

    readTextFile("function?json={\"NextRepeat\":1,\"NextRepeatCondition\":" + id + ",\"NextRepeatNumber\":" + btnId + "}", function (callback) {
        var data = JSON.parse(callback);
        document.getElementById("output").appendChild(alert_message(JSON.stringify(callback)));
        var time = data.times;
        // var time=28923;
        var hour = Math.trunc(time / 3600);
        var minute = Math.trunc(time % 3600 / 60);
        var second = Math.trunc(time % 60);
        time = hour + ":" + minute + ":" + second;
        setHTML("NextRepeat" + data.Number, time);
        //setHTML("NextRepeat0", time);
    })
}

function SignalChange(btnId) {

    var choised = getVal("bySignal" + btnId);
    var result_srting = "";
    if ((choised === signals[2]) || (choised === signals[3] || (choised === signals[4]))) {
        result_srting += "<input class='form-control' id='by_pwm" + btnId + "' type='text' value='' size='1'>";
    }
    if (choised === signals[4]) {
        document.getElementById("output_test").append(alert_message("установить в поле делитель то число, на которое будет делится эти данные", 1000))
    }
    //document.getElementById("inputSignalChange" + btnId).innerHTML = result_srting;
    setHTML("inputSignalChange" + btnId, result_srting)
}

function SelectedLoad(btnId) {
    for (i = 0; i < (btnId + 1); i++) { //сперва сохраняем весь выбор в переменную
        setVal("enabled" + i, selected_id_enabled[i]);
        setVal("timerField" + i, selected_timerField[i]);
        setVal("timerType" + i, selected_timerType[i]);
        setVal("bySignal" + i, selected_bySignal[i]);
        setVal("typeCondition" + i, selected_typeCondition[i]);
        setVal("typeAct" + i, selected_typeAct[i]);
        setVal("typePins" + i, selected_typePins[i]);
        setVal("typeActBtn" + i, selected_typeActBtn[i]);
        setVal("typeActChangeBtn" + i, selected_typeActChangeBtn[i]);
        setVal("by_pwm" + i, selected_by_pwm[i]);
        setVal("times" + i, selected_time[i]);
        setVal("dates" + i, selected_date[i]);
        setVal("pwmTypeAct" + i, selected_pwmTypeAct[i]);
        //typePinsPIN typePinsVAL typePinsIP
        setVal("typePinsPIN" + i, typePinsPIN[i]);
        setVal("typePinsVAL" + i, typePinsVAL[i]);
        setVal("typePinsIP" + i, typePinsIP[i]);

        setVal("typePinsDuration" + i, typePinsDuration[i]);
        setVal("typePinsTimeChoise" + i, typePinsTimeChoise[i]);
        setVal("typePinsrepeats" + i, typePinsrepeats[i]);
        setVal("typeDelay" + i, typeDelay[i]);

        setVal("typePins_minTemp" + i, typePins_minTemp[i]);
        setVal("typePins_maxTemp" + i, typePins_maxTemp[i]);
        setVal("typePins_slaider" + i, typePins_slaider[i]);


    }


}


function SelectedSave(btnId) {
    for (i = 0; i < (btnId + 1); i++) { //сперва сохраняем весь выбор в переменную
        selected_id_enabled[i] = getVal("enabled" + i);
        selected_timerField[i] = getVal("timerField" + i);
        selected_by_pwm[i] = getVal("by_pwm" + i);
        selected_time[i] = getVal("times" + i);
        selected_date[i] = getVal("dates" + i);
        selected_timerType[i] = getVal("timerType" + i);
        selected_bySignal[i] = getVal("bySignal" + i);
        selected_typeCondition[i] = getVal("typeCondition" + i);
        selected_typeActBtn[i] = getVal("typeActBtn" + i);
        selected_typeActChangeBtn[i] = getVal("typeActChangeBtn" + i);
        type[i] = types.indexOf(selected_typeCondition[i]);
        selected_typeAct[i] = getVal("typeAct" + i);
        selected_typePins[i] = getVal("typePins" + i);
        act_[i] = act.indexOf(selected_typeAct[i]);
        selected_pwmTypeAct[i] = getVal("pwmTypeAct" + i);

        typePinsPIN[i] = getVal("typePinsPIN" + i);
        typePinsVAL[i] = getVal("typePinsVAL" + i);
        typePinsIP[i] = getVal("typePinsIP" + i);

        typePinsDuration[i] = getVal("typePinsDuration" + i);
        //typePinsTimeChoise[i] = getVal("typePinsTimeChoise" + i);
        typePinsrepeats[i] = getVal("typePinsrepeats" + i);
        typeDelay[i] = getVal("typeDelay" + i);

        typePins_minTemp[i] = getVal("typePins_minTemp" + i);
        typePins_maxTemp[i] = getVal("typePins_maxTemp" + i);
        typePins_slaider[i] = getVal("typePins_slaider" + i);

        //typePinsDuration
        //typePinsTimeChoise
        //typePinsrepeats
    }
}