var max_number_chosed = 12;

function loadlimits() {
    readTextFile("/function?data={\"pin_setup_limits\":\"1\"}", function (callback) {
        max_number_chosed = (callback !== 404 && isNaN(parseInt(callback))) ? 8 : parseInt(callback) || 12;
        Activation_check();
    });
    document.getElementById("btmBtns").appendChild(bottomButtons());
}

document.addEventListener("DOMContentLoaded", loadlimits);

var inputOption = [], inputPinmode = [], inputPage = [], inputWidjet = [], inputPin = [], inputId = [];
var description;
inputPin = ["no", 0, 2, 4, 5, 10, 12, 13, 14, 15, 16, 17, 1, 3];
var outputPin = [4, 5, 10, 12, 13, 14, 15, 16, 17];

inputPinmode[0] = "no";
inputPinmode[1] = "in";
inputPinmode[2] = "out";
inputPinmode[3] = "PWM";
inputPinmode[4] = "ADC";
inputPinmode[5] = "low. PWM";
inputPinmode[6] = "DHT 1.1 Temp";
inputPinmode[7] = "power MQ7";
inputPinmode[8] = "DHT 1.1 Mist";
inputPinmode[9] = "remote http";
inputPinmode[10] = "power meter";
inputPinmode[11] = "as5600(Variant)";
inputPinmode[12] = "MAC adress";
inputPinmode[13] = "EncA";
inputPinmode[14] = "EncB";
inputPinmode[15] = "ads1117";
inputPinmode[16] = "ds18b20";


inputWidjet[0] = 'unknown';
inputWidjet[1] = 'switch';
inputWidjet[2] = 'button';
inputWidjet[3] = 'progress';
inputWidjet[4] = 'progress-bar';
inputWidjet[5] = 'chart';
inputWidjet[6] = 'data';


const savedValues = {
    one_string: [],
    string_page: [],
    string_descr: [],
    string_widget: [],
    string_pin: [],
    string_id: [],
    string_defaultVal: [],
    string_delimeterVal: [],
    string_IR: [],
    analogDivider: null,
    analogSubtracter: null,
    router: null,
    w433: null
};


var string_delimeterVal_saved = [];
var string_IR_saved = [];
var numberChosed = 0;
var Activation;

function getAvailablePins(mode) {
    switch (mode) {
        case "in":
            return [5, 4, 14, 12, 13];
        case "out":
            return [16, 5, 4, 0, 2, 14, 12, 13, 15, 3, 1];
        case "PWM":
            return pwmPins;
        case "adc":
            return adcPins;
        default:
            return [];
    }
}

function Activation_check() {
    readTextFile('/function?data={\"Activation\":\"0\"}', function (callback) {//проверить если активирован
        if (parseInt(callback) === 1) {//Activated
            Activation = 1;
        } else {//не активирован
            Activation = 0;
            setActivationRequired();
        }

        loadIR();
    });
}

function setActivationRequired() {
    for (var i = 3; i < 12; i++) {
        inputPinmode[i] += "* activation required";
    }
}

var jsonStr = {};
var jsonStrIR = {};
var dataOther = {};
var optionsIR_array = [];

///////////////////////////////////////////////////


////////////////////////////////////////////////////
var openFile = function (event) {
    var input = event.target;
    var reader = new FileReader();
    reader.onload = receivedText;
    reader.readAsText(input.files[0]);
};

function receivedText(e) {
    lines = e.target.result;
    var newArr = JSON.parse(lines);
}

function load2() {
    var data = {};

    readTextFile("pin_setup.txt", function (text) {
        if (text === null) {
            return;
        }
        try {
            data = JSON.parse(text);
            setHTML("test", JSON.stringify(data));
        } catch (e) {
            setHTML("test", getHTML("test") + e + text);
        }
        jsonStr = data;
        makeInputs();
        loadBodyFetchData(jsonStr);//должен быть выше set, иначе меняет все значения value
        set(data);

        makeDivider();
        setRouterPin();

    });

}

////////////////////////////////
function loadBodyFetchData(jsonStr) {
    ///////////////////////////////////////////////
    var data2 = jsonStr;
    var data = document.getElementsByTagName('body')[0].innerHTML;
    var new_string;
    for (var key in data2) {
        new_string = data.replace(new RegExp('{{' + key + '}}', 'g'), data2[key]);
        data = new_string;
    }
    document.getElementsByTagName('body')[0].innerHTML = new_string;

    ////////////////////////////////////
}

function callback_Other(text) {
    if (text === null) {
        load2();
        return;
    }
    dataOther = JSON.parse(text);
    load2();
}

function loadIR() {
    var data = {};
    readTextFile("IRButtons.txt", function (text) {
        readTextFile("other_setup.txt", callback_Other);
        if ((text === null) || (text === 404)) {
            return;
        }
        if (!testJson(text)) return;
        try {
            data = JSON.parse(text);
        } catch (e) {
            document.getElementById("test").innerHTML += e;
        }
        jsonStrIR = data;
        optionsIR_array = (jsonStrIR.name) ? jsonStrIR.name : optionsIR_array[0] = "none";
        optionsIR_array.unshift("none");
    });
}

function makeInputs() {
    var numberChosed;
    if (getVal("sel1") === -1) {
        jsonStr.numberChosed !== undefined ? numberChosed = jsonStr.numberChosed : 0;
    } else {
        numberChosed = getVal("sel1") !== null ? getVal("sel1") : 0;
    }
    set(null);

    number_buttons = max_number_chosed;
    var array_selected = [];
    for (i = 0; i <= number_buttons; i++) {
        array_selected[i] = i;
    }
    setHTML("sel1div", makeinOption(array_selected, "sel1", "makeInputs()"));
    setVal("sel1", numberChosed);
}

function set(jsonStr) {
    numberChosed = getVal("sel1");
    if (jsonStr) {
        numberChosed = jsonStr.numberChosed;
        setVal("sel1", numberChosed);
    }
    saveAllStrings(numberChosed);
    var description = "";
    numberInputOptions = 2;
    var result_table =
        "<tr>" +
        "<td >one_st</td>" +

        "<td>descr</td>" +
        "<td>widget</td>" +
        "<td>pinout</td>" +
        "<td>IR</td>" +
        "<td>def</td>" +
        "<td>cond</td>" +
        "</tr>";
    for (var i1 = 0; i1 < numberChosed; i1++) {
        btnId = i1;
        result_table += "<tr id='tr" + btnId + "'>" +
            "<td id='Inputs_table" + btnId + "'></td>" +
            "<td id='Inputs_descr_table" + btnId + "'></td>" +
            "<td id='Inputs_widget_table" + btnId + "'></td>" +
            "<td id='Inputs_pin_table" + btnId + "'></td>" +
            "<td id='Inputs_IR_table" + btnId + "'></td>" +
            "<td id='Inputs_defaultVal_table" + btnId + "'></td>" +
            "<td id='condition_table" + btnId + "'></td>" +
            "<td id='MQTT_adress" + btnId + "'></td>" +
            "</tr>";
    }


    setHTML("table", getHTML("table") + "<tbody id='body_table'><tbody>");
    setHTML("body_table", result_table);


    for (var i = 0; i < numberChosed; i++) {
        if (jsonStr) {
            inputId = jsonStr.id;
            description = jsonStr.descr[i];
            defaultVal = jsonStr.defaultVal[i];

            string_delimeterVal_saved[i] = 0;
            if (jsonStr.delimVal) {
                string_delimeterVal_saved[i] = jsonStr.delimVal[i];
                if (typeof jsonStr.delimVal[i] === 'undefined' || jsonStr.delimVal[i] === null) {
                    string_delimeterVal_saved[i] = 0;
                }
            }
        } else {
            description = "button:" + i;
            defaultVal = 0;
        }


        var MQTT_adress;
        if (jsonStr) {
            if (dataOther) {
                MQTT_adress = "MQTT topic: " + dataOther.deviceID + "/" + description + "/" + i + "\n";
                MQTT_adress += "MQTT control topic: " + dataOther.deviceID + "/" + description + "/" + i + "/" + "status" + "\n";
                MQTT_adress += "remote control HTTP: " + window.location.host + "/aRest?data={pin:\"" + inputPin.indexOf(parseInt(jsonStr.pin[i])) + "\",\"val\":\"1\"}";
            }
        }


        setHTML("Inputs_table" + i, makeinOption(inputPinmode, "one_string" + i, "choisedPinmode(" + i + ", true)"));
        setHTML("Inputs_descr_table" + i, "<input type='text' class='form-control' id='string_descr" + i + "' value='" + description + "'>");
        setHTML("Inputs_widget_table" + i, makeinOption(inputWidjet, 'string_widget' + i, false));
        setHTML("Inputs_IR_table" + i, makeinOption(optionsIR_array, 'string_IR' + i, false));
        setHTML("Inputs_pin_table" + i, makeinOption(inputPin, 'string_pin' + i, false));
        document.getElementById("Inputs_defaultVal_table" + i).innerHTML += "<input type='text' class='form-control' id='string_defaultVal" + i + "'value='" + defaultVal + "'</input>";
        if (i < 3) {
            document.getElementById("condition_table" + i).innerHTML += "<a  class='btn btn-primary btn-xs' href=/condition.htm?id='>:" + i + "</a><br>";
        }
        setHTML("MQTT_adress" + i, "<div title='" + MQTT_adress + "'><p>?</p></div>");
        if (jsonStr) {
            saveAllStrings(numberChosed);
        }
    }


    if (jsonStr) {
        for (var i = 0; i < numberChosed; i++) {
            setVal("one_string" + i, inputPinmode[jsonStr.pinmode[i]]);
            setVal("string_widget" + i, inputWidjet[jsonStr.widget[i]]);
            setVal("string_pin" + i, jsonStr.pin[i] = jsonStr.pin[i] === 255 ? inputPin[0] : jsonStr.pin[i]);
        }
        saveAllStrings(numberChosed);
    }

    loadAllStrings(numberChosed);
    loadAllStrings(numberChosed);


}

function makeIRsetup() {
    var ir_string = "<table class='table'>";
    var AviailablePinsT = inputPin.slice();
    AviailablePinsT.unshift(inputPin[-1]);
    var AviailablePinsR = inputPin.slice();
    AviailablePinsR.unshift(inputPin[-1]);
    AviailablePinsR.splice(AviailablePinsR.indexOf(0), 1);
    var optionsIR_receive_output = makeinOption_pin(AviailablePinsR, jsonStr.IR_rec ? jsonStr.IR_rec : 0);
    var optionsIR_output_output = makeinOption_pin(AviailablePinsT, jsonStr.IR_LED ? jsonStr.IR_LED : 0);
    ir_string += "<tr><td>IR receiver</td><td><select class='form-control' id='IR_receive_output'>" + optionsIR_receive_output + "</select></td></tr>";
    ir_string += "<tr><td>IR LED</td><td><select class='form-control' id='IR_output_output'>" + optionsIR_output_output + "</select></td></tr></table>";
    setHTML("IR_setup", ir_string);
    IR_receive_output = jsonStr.IR_rec ? jsonStr.IR_rec : 0;
    IR_output_output = jsonStr.IR_LED ? jsonStr.IR_LED : 0;
    setVal("IR_receive_output", IR_receive_output);
    setVal("IR_output_output", IR_output_output);
    //var form = document.getElementById("form");

}

var analogDivider = 0;
var analogSubtracter = 0;
var router;

function makeDivider() {
    var divider = "";

    if (jsonStr) {
        analogDivider = !isNaN(parseFloat(jsonStr.aDiv)) ? parseFloat(jsonStr.aDiv) : 1;
        analogSubtracter = !isNaN(parseFloat(jsonStr.aSusbt)) ? parseFloat(jsonStr.aSusbt) : 0;
    }
    divider += "<table class='table' id='Divider_table'><tr><td>analog output divider:</td>";
    divider += "<td><input type='text' class='form-control' id='analogDivider' value='" + analogDivider + "'</input></td></tr>";
    divider += "<tr><td>отнять:</td><td><input type='text' class='form-control' id='analogSubtracter'value='" + analogSubtracter + "'</input></td></tr></table>";
    setHTML("Divider", divider);

}

var w433;

function setRouterPin() {
    ////////////////////////////////////
    var Divider_table = document.getElementById("Divider_table");
    if (Divider_table == null) return;
    var array = inputPin;
    var selectList = document.createElement("select");
    var select433 = document.createElement("select");
    selectList.id = "router";
    select433.id = "w433";
    // selectList.setAttribute("id", "router");
    selectList.className = "form-control";
    select433.className = "form-control";

    Divider_table.appendChild(selectList);
    Divider_table.appendChild(select433);
    if (jsonStr) {
        router = !isNaN(parseInt(jsonStr.router)) ? parseInt(jsonStr.router) : 255;
        w433 = !isNaN(parseInt(jsonStr.w433)) ? parseInt(jsonStr.w433) : 255;
    }

    for (var i = -1; i < array.length; i++) {
        if (array[i] !== undefined) {
            var option = document.createElement("option");
            var option_433 = document.createElement("option");
            if (array[i] == router) {
            }
            option.text = array[i];
            option_433.text = array[i];
            selectList.appendChild(option);
            select433.appendChild(option_433);
        }
    }
    var row = Divider_table.insertRow(0);
    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);

    var row433 = Divider_table.insertRow(0);
    var cell1_433 = row433.insertCell(0);
    var cell2_433 = row433.insertCell(1);

    selectList.value = router === 255 ? "no" : router;
    select433.value = w433 === 255 ? "no" : w433;
    cell2.appendChild(selectList);
    cell1.innerHTML = "pin router";

    cell2_433.appendChild(select433);
    cell1_433.innerHTML = "w433 pin";
    ////////////////////////////////////
}
function choisedPinmode(i, makeInput) {
    if (document.getElementById("one_string" + i)) {
        if (makeInput) {
            makeInputs();
        }
        choised = getVal("one_string" + i);
        setHTML(`Inputs_pin_table${i}`, makeinOption(getAvailablePins(choised), `string_pin${i}`, false));
        switch (choised) {
            case inputPinmode[2]: //out
                break;
            case inputPinmode[3]: //pwm
                setVal("string_widget" + i, inputWidjet[3]); //range
                setDisable("string_widget" + i);
                break;
            case inputPinmode[4]: //adc
                setVal("string_widget" + i, inputWidjet[4]); //small-badge
                setVal("string_pin" + i, 17); //17
                if (inputWidjet[jsonStr.widget[i]]) {
                    setVal("string_widget" + i, inputWidjet[jsonStr.widget[i]]); //small-badge
                }
                break;
            case inputPinmode[5]: //low_pwm
                if (document.getElementById("string_widget" + i)) {
                    document.getElementById("string_widget" + i).value = inputWidjet[3]; //range
                }
                break;
            case inputPinmode[9]:
                setVal("string_descr" + i, "192.168.1.108/aRest?data={stat:2,val:1}");
                break;
            case inputPinmode[100]: //IR
                if (typeof jsonStrIR.name === "undefined") {
                    value = "<a class='btn btn-block btn-primary' href='\IR_setup'>Настройка IR</a>";
                    setHTML("Inputs_descr_table" + i, value);
                } else {
                    descr_new = "<select class='form-control' onchange='selectidIR(" + i + ")' id='string_descr" + i + "'></select>";
                    make_string_pin();
                    descr_new += "<a href='\IR_setup'>IR</a>";
                    setHTML("Inputs_descr_table" + i, descr_new);
                    value = makeinOption_pin(jsonStrIR.name, 0);
                    setHTML("string_descr" + i, value);
                    setVal("string_widget" + i, inputWidjet[2]); //button
                    document.getElementById("string_widget" + i).disabled = true;
                    document.getElementById("string_defaultVal" + i).disabled = true;
                    setDisable("string_id" + i);
                    setDisable("string_pin" + i);
                    var id_my = [];
                    for (i1 = 0; i1 < jsonStrIR.name.length; i1++) {
                        id_my[i1] = i1;
                    }
                    value2 = makeinOption_pin(id_my, jsonStrIR.name.indexOf(getVal("string_descr" + i)));
                    setHTML("string_pin" + i, value2);
                    selectidIR(i);
                }
                break;
            case inputPinmode[11]: //IR
                setVal("string_pin" + i, "no"); //17
                break;
            case inputPinmode[12]: //IR
                setVal("string_descr" + i, "ENTER MAC");
                document.getElementById("output").appendChild(alert_message("enter ENTER MAC in description"));
                break;
            case inputPinmode[15]: //ads
                document.getElementById("output").appendChild(alert_message("enter Number pin in Default"));
                break;
            case inputPinmode[16]: //ds18b20
                document.getElementById("output").appendChild(alert_message("enter number by index in Default, connect ds18b20 to GPI02(D4)", 30));
                break;
        }

    }
}

function setDisable(ID) {
    if (document.getElementById(ID)) {
        document.getElementById(ID).disabled = true;
    } else {
        if (document.getElementById("test")) {

        }
    }
}

function selectidIR(i) { //сделать выбор изjsonStr.pin в string_descr5
    index = 0;
    index = document.getElementById("string_descr" + i).selectedIndex;
    setVal("string_defaultVal" + i, jsonStrIR.code[index]);
    document.getElementById("string_pin" + i).selectedIndex = document.getElementById("string_descr" + i).selectedIndex;
}

function saveAllStrings(numberChosed) {
    for (let i = 0; i < numberChosed; i++) {
        savedValues.one_string[i] = getVal(`one_string${i}`);
        savedValues.string_page[i] = getVal(`string_page${i}`);
        savedValues.string_descr[i] = getVal(`string_descr${i}`);
        savedValues.string_widget[i] = getVal(`string_widget${i}`);
        savedValues.string_pin[i] = getVal(`string_pin${i}`);
        savedValues.string_id[i] = getVal(`string_id${i}`);
        savedValues.string_defaultVal[i] = parseInt(getVal(`string_defaultVal${i}`));
        savedValues.string_delimeterVal[i] = getVal(`string_delimeterVal${i}`) === -1 ? 0 : getVal(`string_delimeterVal${i}`);
        savedValues.string_IR[i] = getVal(`string_IR${i}`);
    }
    savedValues.analogDivider = getVal("analogDivider");
    savedValues.analogSubtracter = getVal("analogSubtracter");
    savedValues.router = getVal("router");
    savedValues.w433 = getVal("w433");
}

function loadAllStrings(numberChosed) {
    for (let i = 0; i < numberChosed; i++) {
        setVal(`one_string${i}`, savedValues.one_string[i]);
        setVal(`string_page${i}`, savedValues.string_page[i]);
        setVal(`string_descr${i}`, savedValues.string_descr[i]);
        setVal(`string_widget${i}`, savedValues.string_widget[i]);
        setVal(`string_pin${i}`, savedValues.string_pin[i]);
        setVal(`string_id${i}`, savedValues.string_id[i]);
        setVal(`string_defaultVal${i}`, savedValues.string_defaultVal[i]);
        setVal(`string_delimeterVal${i}`, savedValues.string_delimeterVal[i]);
        setVal(`string_IR${i}`, savedValues.string_IR[i]);
    }
    setVal("analogDivider", savedValues.analogDivider);
    setVal("analogSubtracter", savedValues.analogSubtracter);
    setVal("router", savedValues.router);
    setVal("w433", savedValues.w433);
}

function makeinOption_pin(inputOption, choosed) {
    var options = "";
    var i1;
    var selected = "";
    for (i1 = -1; i1 < inputOption.length; i1++) {
        if (i1 == choosed) {
            selected = " selected";
        } else {
            selected = "";
        }
        if (inputOption[i1] != null) {
            options += "<option" + selected + ">" + inputOption[i1] + "</option>";
        }
    }
    return options;
}
