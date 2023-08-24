/**
 * Created by Башня1 on 17.02.2019.
 */
document.addEventListener("DOMContentLoaded", load);
var Data_limits = {
    NUM_LEDS: 150,
    NUM_CHARTS: 4,
    DATA_PIN: 0,
    NUMBER_SAVE_FILES: 5
};

function loadAll8211(val) {

}

var Data_all_settings = [{}];

function load() {
    main_ = document.getElementById("main_");
    tbl = document.getElementById('table');


    header_ = document.getElementById("header_");
    createTable();
    loadAddButtion();
    loadSecondTable();
    setHTML("btmBtns", bottomButtons());
    loadMainJson();


    var val = 0;
    // loadMainJson();

    var load_only_once = 0;

    function load8211(val) {
        readTextFile("ws8211/" + val + ".txt", function (callback) {
            //.if(testJson(callback)){
            if (testJson(callback)) {
                Data_all_settings[val] = callback;
            }
            main_.appendChild(alert_message(callback + ": " + val));
            if (val < Data_limits.NUMBER_SAVE_FILES) {
                val++;
                load8211(val);
            } else {
                //main_.appendChild(alert_message(callback+val));
                load_only_once === 0 ? loadSettings(Data_all_settings[getVal("NumbersSave")]) : load_only_once = 1;
            }
        });
    }

    function loadMainJson() {
        readTextFile("/function?json={'ws2811_setup':1}", function (callback) {
            if (testJson(callback)) {

                Data_limits = JSON.parse(callback);
                Data_limits.NUMBER_SAVE_FILES = 5;
                document.getElementById("output").appendChild(alert_message(JSON.stringify(callback)));


            }
            load8211(val);

            //alert(callback);
            /*
             try {
             readTextFile("ws2811_setup.txt", function (json) {
             try {
             loadSettings(json);
             } catch (e) {
             setHTML("input", getHTML("input") + e);
             }
             });
             } catch (e) {

             }
             */
        })


    }

}

var main_;
var tbl, tbl2;
var numberElements = 0;
var Elements = {};

function loadAddButtion() {

    var btn = document.createElement("BUTTON");        // Create a <button> element
    btn.innerHTML = 'ADD';
    btn.onclick = function () {
        add_btn(-1);
        return false;
    };                          // Append the text to <button>
    btn.className = "form-control";
    main_.appendChild(btn);
////BtnSave////////////////
    var btnSave = document.createElement("BUTTON");        // Create a <button> element
    btnSave.innerHTML = 'Save';
    btnSave.id = "BtnSave";
    btnSave.onclick = function () {
        Save_ws8211(getVal("NumbersSave"));
        return false;
    };                          // Append the text to <button>
    btnSave.className = "form-control";
    main_.appendChild(btnSave);
////numberButtons/////////////////////
    //var SelectSave = document.createElement("select");
    var optionNumbers = [3];
    for (var i = 0; i < Data_limits.NUMBER_SAVE_FILES; i++) {
        optionNumbers[i] = i;
    }
    //main_.insertBefore(optionNumbersP, main_.firstChild);
    var select2 = makeinOption_child(optionNumbers, "NumbersSave", function () {
//загружаем текущее состояние
        //clearTimeOutSave();
        //send_New_values_to_ESP();
        setHTML("BtnSave", "Save " + getVal("NumbersSave"));
        loadSettings(Data_all_settings[getVal("NumbersSave")]);
    });


    header_.insertBefore(select2, header_.firstChild);
}

function Save_ws8211(val) {
    //setVal("input",jsonOut);
    //var jsonOut = send_New_values_to_ESP();
    main_.appendChild(alert_message(JSON.stringify(jsonOut)));
    saveData("ws8211/" + val + ".txt", jsonOut, function (callback) {
        //alert(callback);
        main_.appendChild(alert_message(callback));
        Data_all_settings[val]=jsonOut;
    });

}

function loadSettings(json) {

    //document.getElementById("table").innerHTML="";
    var Parent = document.getElementById("table");
    while (Parent.hasChildNodes()) {
        Parent.removeChild(Parent.firstChild);
    }
    if (!testJson(json)) return;
    // var json =
    //   "{\"from\":[0,10],\"to\":[10,20],\"type\":[4,3],\"dir\":[1,0],\"col\":[0,1],\"num\":2}";
    //readTextFile("ws2811_setup", callback_load_ws2811);

    jsonObject = JSON.parse(json);
    numberElements = (jsonObject.num !== undefined) ? jsonObject.num : 0;

    // numberElements=1;
    // numberElements = 0;
    setVal("sp_ws8211", jsonObject.sp);
    setVal("duration", jsonObject.dr);
    setVal("fade", jsonObject.fd);
    setVal("bright", jsonObject.br);
    setVal("reverse_set", jsonObject.r);
    setVal("inv", jsonObject.inv);

    for (var i = 0; i < numberElements; i++) {
        //if(getVal())
        if (document.getElementById("tr" + i) === null) {
            //   main_.appendChild(alert_message(getVal("tr" + i)))
            add_btn(i);
        }
    }
    for (var i = 0; i < numberElements; i++) {
        //alert(getVal("range1" + i));
        setVal("range1" + i, jsonObject.from[i]);
        setVal("range_how" + i, jsonObject.to[i] - jsonObject.from[i]);
        setVal("type_id" + i, typesArray[jsonObject.type[i]]);
        setVal("dir_id" + i, dir_array_text[jsonObject.dir[i]]);
        setVal("range_color" + i, jsonObject.col[i]);
        setVal("wh" + i, jsonObject.wh[i]);
        setVal("brightRange" + i, jsonObject.br_[i]);
        //setVal("type_id"+i,"back");
        //var element = document.getElementById("type_id"+i);
        //element.value = "back";
    }
    //numberElements=jsonObject.num;
    clearTimeOutSave();
    run();
}

function delBtn(id) {
    //clearTimeOutSave();
    //document.getElementById("table").deleteRow("a"+id);
    document.getElementById("table").deleteRow("tr" + id);
    // var div = document.createElement('div');
    // div.className = "alert alert-success";
    // div.innerHTML = id;

    //main_.appendChild(div);

    // setVal("tr" + id, "");
}

var timeOut_saveFile;

function saveFileSettingsWs2811() {
    //alert("saveFileSettingsWs2811");
    //setHTML("output",getHTML("output")+ saveData("ws2811_setup.txt",JSON.stringify(jsonOut)));

    saveData("ws2811_setup.txt", jsonOut, function (callback) {
        var div = document.createElement('div');
        div.className = "alert alert-success";
        div.innerHTML = callback;
        div.id = "ok_message";
        setTimeout(function () {
            div.remove();
        }, 1000);
        main_.appendChild(div);
    });


    //if (saveData !== null) {

    //}

}

function clearTimeOutSave() {
    //alert("clearTimeOutSave");
    send_New_values_to_ESP();
    //clearTimeout(timeOut_saveFile);
    //timeOut_saveFile = setTimeout(saveFileSettingsWs2811, 5000);
}

function OptionChange() {
    clearTimeOutSave();
}

function add_btn(num) {
    var i = num;
    if (num === -1) {//вручную
        i = numberElements;
    }
    //if(jsonOut.num)
    var act_elemens = 0;
    while (act_elemens < numberElements) {
        if (getVal("range1" + i) !== undefined) act_elemens++;
    }

    if (act_elemens > Data_limits.NUM_CHARTS) {
        // return;
    }


    var iDiv = document.createElement('div');
    iDiv.className = 'input-group mb-3';
    var aDiv = document.createElement('div');
    aDiv.className = 'input-group-prepend';
    aDiv.appendChild(iDiv);

    var idtable = document.createElement('p');
    idtable.innerHTML =
        "<p1 id='idTable" + i + "'></p1>";


    var fromRange = document.createElement('p');
    //fromRange.appendChild(aDiv);
    //fromRange.className = "form-control";//range From
    fromRange.innerHTML =
        "<p id=\"\">от</p>" +
        "<p id='rangeText" + i + "'></p>" +
        "<input id='range1" + i + "' " +
        "class='form-control'" +
        "type='range' min='0' max='" + Data_limits.NUM_LEDS + "' step='1' " +
        "onchange='clearTimeOutSave()'/>";
    //fromRange.onchange=clearTimeOutSave();

    var toRange = document.createElement('p');
    toRange.innerHTML =
        "<p1 id='range_to_Text" + i + "'></p1>" +
        "<br>" +
        "class='form-control'" +
        "type='range' min='0' max='" + Data_limits.NUM_LEDS + "' step='1'" +
        "onchange='clearTimeOutSave()' />";

    var howRange = document.createElement('p');
    //howRange.className = "form-control";
    howRange.innerHTML =
        "<p id=\"\">сколько</p>" +
        "<p1 id='range_how_Text" + i + "'></p1>" +
        "<input id='range_how" + i + "' " +
        "class='form-control'" +
        "type='range' min='0' max='" + (Data_limits.NUM_LEDS - getVal("range1" + i)) + "' step='1'" +
        "onchange='clearTimeOutSave()' />";

    var colorRange = document.createElement('p');
    colorRange.innerHTML =
        "<p id=\"\">цвет</p>" +
        "<p1 id='range_color_Text" + i + "'></p1>" +
        "<input id='range_color" + i + "' " +
        "class='form-control'" +
        "type='range' min='0' max='" + 255 + "'step='1'" +
        "onchange='clearTimeOutSave()' />";
    //toRange.onchange=clearTimeOutSave();
    var brightRange = document.createElement('p');
    brightRange.innerHTML =
        "<p id=\"\">яркость</p>" +
        "<p1 id='brightRange_Text" + i + "'></p1>" +
        "<input id='brightRange" + i + "' " +
        "class='form-control'" +
        "type='range' min='0' max='" + 255 + "'step='1'" +
        "onchange='clearTimeOutSave()' />";
    var wh = document.createElement('p');
    wh.innerHTML =
        "<p id=\"\">белый</p>" +
        "<p1 id='wh_Text" + i + "'></p1>" +
        "<input id='wh" + i + "' " +
        "class='form-control'" +
        "type='range' min='0' max='" + 255 + "'step='1'" +
        "onchange='clearTimeOutSave()' />";
    //toRange.onchange=clearTimeOutSave();


    var type = document.createElement('p');
    type.innerHTML = "тип" + makeinOption(typesArray, "type_id" + i, 'clearTimeOutSave()');

    var dir = document.createElement('p');
    dir.innerHTML = "напр" + makeinOption(dir_array_text, "dir_id" + i, 'clearTimeOutSave()');


    var DeleteBtn = document.createElement('p');
    DeleteBtn.id = "delete" + i;
    DeleteBtn.innerHTML = "<input id='btn+" + i + "' class='btn btn-block btn-success' type='button' value='X' onclick='delBtn(" + i + ")' /></br>";


    var tr = document.createElement('tr');
    tr.id = "tr" + i;//зеленая аптека для чувствительной кожи

    var tr1 = createTR(tr);
    tr1.id = tr.id;
    //fromRange.appendChild(toRange);
    /*
     var toTr = createTR(tr);
     createTD(toTr, toRange);
     tbl.appendChild(toTr);
     */
    //fromRange.appendChild(type);


    //fromRange.appendChild(iDiv);
    //document.getElementsByTagName('body')[0].appendChild(iDiv);
    //var div2 = document.createElement('div');
    //div2.className = 'block-2';
    //iDiv.appendChild(div2);


    createTD(tr, idtable);
    createTD(tr1, fromRange);
    createTD(tr1, howRange);
    createTD(tr1, type);
    createTD(tr1, dir);
    createTD(tr1, DeleteBtn);

    var tr2 = createTR(tr);
    tr2.id = tr.id;
    createTD(tr2, colorRange);
    createTD(tr2, brightRange);
    createTD(tr2, wh);
    tbl.appendChild(tr);
    tr.appendChild(tr1);
    tr.appendChild(tr2);


    //tableDiv=document.getElementById("table");
    //tableDiv.appendChild(tableBody);

    if (num === -1) {//вручную
        numberElements++;
    }
    //numberElements++;
}

function loadSecondTable() {

    var sp_ws8211 = document.createElement('p');
    sp_ws8211.innerHTML =
        "<p id=\"\">скорость</p>" +
        "<p1 id='sp_ws8211_Text'></p1>" +
        "<input id='sp_ws8211'" +
        "class='form-control'" +
        "type='range' min='1' max='255' step='1' " +
        "onchange='clearTimeOutSave()'/>";
    var duration = document.createElement('p');
    duration.innerHTML =
        "<p id=\"\">duration</p>" +
        "<p1 id='duration_Text'></p1>" +
        "<input id='duration'" +
        "class='form-control'" +
        "value = '255' " +
        "type='range' min='0' max='255' step='1' " +
        "onchange='clearTimeOutSave()'/>";
    var fade = document.createElement('p');
    fade.innerHTML =
        "<p id=\"\">затухание</p>" +
        "<p1 id='fade_Text'></p1>" +
        "<input id='fade'" +
        "class='form-control'" +
        "type='range' min='1' max='100' step='1' " +
        "onchange='clearTimeOutSave()'/>";
    ArrayFadetype = [
        'nscale8_video',
        'fade_video',
        'fadeLightBy',
        'fadeToBlackBy',
        'fade_raw',
        'nscale8_raw',
        'nscale8',
        'fadeUsingColor',
        'blur1d'];
    var fadetype = document.createElement('p');
    fadetype.innerHTML =
        "<p id=\"\">режим затухания</p>" +
        "<p1 id='fadetype_Text'></p1>" +
        makeinOption(ArrayFadetype, "fadetype", "clearTimeOutSave()");

    var bright = document.createElement('p');
    bright.innerHTML =
        "<p id=\"\">яркость</p>" +
        "<p1 id='bright_Text'></p1>" +
        "<br>" +
        "<input id='bright'" +
        "class='form-control'" +
        "type='range' min='0' max='255' step='1' " +
        "onchange='clearTimeOutSave()'/>";

    var reverse_set = document.createElement('p');

    reverse_set.className = "form-control";
    reverse_set.innerHTML =
        "возврат<input id='reverse_set'" +
        "class='form-control'" +
        "type='checkbox'" +
        "onchange='clearTimeOutSave()'/>";

    var inv = document.createElement('p');
    inv.className = "form-control";
    inv.innerHTML =
        "inv<input id='inv'" +
        "class='form-control'" +
        "type='checkbox'" +
        "onchange='clearTimeOutSave()'/>";

    // var tr = document.createElement("tr");
    tbl2 = document.getElementById('table2');
    var tr = document.createElement('tr');
    createTD(tr, sp_ws8211);
    createTD(tr, duration);
    createTD(tr, fade);
    createTD(tr, fadetype);

    createTD(tr, bright);

    var tr1 = document.createElement('tr');
    createTD(tr1, reverse_set);
    createTD(tr1, inv);
    tbl2.className = "table";
    tbl2.style.width = '100%';
    tbl2.setAttribute('border', '1');
    tbl2.appendChild(tr);
    tbl2.appendChild(tr1);

    setVal("fadetype", "fadeToBlackBy");
}

var fromArray = [];
var toArray = [];
var type_array = [];
var dir_array = [];
var colorArray = [];
var white_col_ = [];
var bright_array_=[];

var typesArray = ["в сторону", "с двух", "в цвете", "середина", "RND", "через одну", "на три", "радуга", "радуга2", "конфети", "sinelon", "bpm", "juggle", "glow max", "случайно", "цветомузыка"];
var dir_array_text = [">", "<", "RND", "0"];
var jsonOut;

function run() {
    //send_New_values_to_ESP();
    //timeOut = setTimeout(run, 10000);

}

function send_New_values_to_ESP() {
    jsonOut = {};
    //jsonOut.num = numberElements;
    var json =
        "{\"from\":[0],\"to\":[40],\"type\":[2],\"dir\":[4],\"col\":[1],\"num\":1,\"sp\":10,\"fd\":200,\"br\":2}";

    //jsonOut.sp=
    var act_elements = 0;
    for (var i = 0; i < numberElements; i++) {
        if (getVal("range1" + i) !== -1) {
            fromArray[act_elements] = parseInt(getVal("range1" + i));
            //howArray[act_elements]=parseInt(getVal("range_to" + i))+parseInt(getVal("range_to" + i));
            //toArray[act_elements] = parseInt(getVal("range_to" + i));
            toArray[act_elements] = parseInt(getVal("range_how" + i)) + fromArray[act_elements];
            type_array[act_elements] = typesArray.indexOf(getVal("type_id" + i));
            dir_array[act_elements] = dir_array_text.indexOf(getVal("dir_id" + i));
            colorArray[act_elements] = parseInt(getVal("range_color" + i));
            white_col_[act_elements] = parseInt(getVal("wh" + i));
            bright_array_[act_elements] = parseInt(getVal("brightRange"+i));

            setHTML("rangeText" + i, fromArray[act_elements]);
            setHTML("range_to_Text" + i, toArray[act_elements]);
            setHTML("range_how_Text" + i, toArray[act_elements] - fromArray[act_elements]);
            setHTML("range_color_Text" + i, parseInt(getVal("range_color" + i)));
            setHTML("brightRange_Text" + i, parseInt(getVal("brightRange" + i)));
            setHTML("wh_Text" + i, parseInt(getVal("wh" + i)));

            setHTML("sp_ws8211_Text", parseInt(getVal("sp_ws8211")));
            setHTML("duration_Text", parseInt(getVal("duration")));
            setHTML("fade_Text", parseInt(getVal("fade")));
            setHTML("bright_Text", parseInt(getVal("bright")));


            setNewlimits(i, act_elements);

            //range_how.value=
            act_elements++;
        } else {

            fromArray.splice(i, 1);
            toArray.splice(i, 1);
            type_array.splice(i, 1);
            dir_array.splice(i, 1);
            colorArray.splice(i, 1);
            white_col_.splice(i, 1);
            bright_array_.splice(i,1);

        }
        fromArray.splice(act_elements);
        toArray.splice(act_elements);
        type_array.splice(act_elements);
        dir_array.splice(act_elements);
        colorArray.splice(act_elements);
        white_col_.splice(act_elements);
        bright_array_.splice(act_elements);

    }
    jsonOut.from = fromArray;
    jsonOut.to = toArray;
    jsonOut.type = type_array;
    jsonOut.dir = dir_array;
    jsonOut.col = colorArray;
    jsonOut.wh = white_col_;
    jsonOut.br_ = bright_array_;

    jsonOut.num = act_elements;
    jsonOut.sp = parseInt(getVal("sp_ws8211"));
    jsonOut.dr = parseInt(getVal("duration"));
    jsonOut.fd = parseInt(getVal("fade"));
    jsonOut.fdt = ArrayFadetype.indexOf(getVal("fadetype"));
    jsonOut.br = parseInt(getVal("bright"));

    getVal("reverse_set") === true ? jsonOut.r = 1 : false;
    getVal("inv") === true ? jsonOut.inv = 1 : false;
    //jsonOut.r=parseInt(getVal("reverse_set")===true?1:0);
    //var send_that_json = JSON.stringify(jsonOut);
    var send_that_json = JSON.stringify(jsonOut);
    setHTML("output", send_that_json);

    readTextFile("/ws2811AJAX?json=".concat(send_that_json), function (callback) {

    });
    return jsonOut;
}

function setNewlimits(i, act_elements) {
    var range_how = document.getElementById("range_how" + i);
    range_how.max = Data_limits.NUM_LEDS - fromArray[act_elements];
    setHTML("idTable" + i, act_elements);
}

function gID(id) {
    if (document.getElementById(id)) {
        return document.getElementById(id)
    }
    return null
}

function createTable() {//ЛОЖКА УНО, КОСМЕТОЛОГИЧЕСКОЕ СИТЕЧКО
    //body = document.getElementById("main_");
    //var body = document.getElementsByTagName('body')[0];
    tbl = document.getElementById('table');
    tbl === null ? alert("no table!") : false;
    tbl.className = "table";
    tbl.style.width = '100%';
    tbl.setAttribute('border', '0');
    //var tbdy = document.createElement('tbody');
    //tbdy.className = "form-control";
    //tbdy.id = "tbody_id";
    /*
     for (var i = 0; i < 3; i++) {
     var tr = document.createElement('tr');
     for (var j = 0; j < 2; j++) {
     if (i == 2 && j == 1) {
     break
     } else {
     var td = document.createElement('td');
     td.appendChild(document.createTextNode('\u0020'))
     i == 1 && j == 1 ? td.setAttribute('rowSpan', '2') : null;
     tr.appendChild(td)
     }
     }
     tbdy.appendChild(tr);
     }
     */
    //tbl.appendChild(tbdy);
    main_.appendChild(tbl);
    //tableBody = document.getElementById("tbody_id");
}
