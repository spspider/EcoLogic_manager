document.addEventListener("DOMContentLoaded", load);
var IRjson = {code: [], name: [], rawID: [], rawCode: [], rawCodeLen: []};
var progress;
var CODE = {};
//  var ModifiedJson;
function load() {
    //alert(typeof HelperLoaded);
    //IRjson = {code: [], name: [], rawID: [], rawCode: [], rawCodeLen: []};
    load2();
    setHTML("btmBtns",bottomButtons());
 }
var a = {
    "Ё": "YO",
    "Й": "I",
    "Ц": "TS",
    "У": "U",
    "К": "K",
    "Е": "E",
    "Н": "N",
    "Г": "G",
    "Ш": "SH",
    "Щ": "SCH",
    "З": "Z",
    "Х": "H",
    "Ъ": "'",
    "ё": "yo",
    "й": "i",
    "ц": "ts",
    "у": "u",
    "к": "k",
    "е": "e",
    "н": "n",
    "г": "g",
    "ш": "sh",
    "щ": "sch",
    "з": "z",
    "х": "h",
    "ъ": "'",
    "Ф": "F",
    "Ы": "I",
    "В": "V",
    "А": "a",
    "П": "P",
    "Р": "R",
    "О": "O",
    "Л": "L",
    "Д": "D",
    "Ж": "ZH",
    "Э": "E",
    "ф": "f",
    "ы": "i",
    "в": "v",
    "а": "a",
    "п": "p",
    "р": "r",
    "о": "o",
    "л": "l",
    "д": "d",
    "ж": "zh",
    "э": "e",
    "Я": "Ya",
    "Ч": "CH",
    "С": "S",
    "М": "M",
    "И": "I",
    "Т": "T",
    "Ь": "'",
    "Б": "B",
    "Ю": "YU",
    "я": "ya",
    "ч": "ch",
    "с": "s",
    "м": "m",
    "и": "i",
    "т": "t",
    "ь": "'",
    "б": "b",
    "ю": "yu"
};

function transliterate(word) {
    return word.split('').map(function (char) {
        return a[char] || char;
    }).join("");
}
function load2() {


    try {
        readTextFile("IRButtons.txt", function (text) {

            try {
                //IRjson={};

                IRjson = JSON.parse(text);
                IRjson.rawID === undefined ? IRjson.rawID = [] : {};
                IRjson.code === undefined ? IRjson.code = [] : {};
                IRjson.name === undefined ? IRjson.name = [] : {};

                makeIRList(IRjson);
                //document.getElementById("codelist").innerHTML += IRjson.code[0];
            } catch (e) {

                makeIRList(IRjson);
                // document.getElementById("test").innerHTML += "JSON.parse: " + e;

            }
        });
    } catch (e) {

        //document.getElementById("test").innerHTML += e;
    }


}
function delCb(xmlHttp, a) {
    return function () {
        if (xmlHttp.readyState == 4) {
            if (xmlHttp.status != 200) {
                //alert("ERROR[" + xmlHttp.status + "]: " + xmlHttp.responseText)
            } else {
                //saveCode();
            }
        }
    }
}
function httpDelete(a) {
    var xmlHttp = createXmlHttpObject();
    xmlHttp.onreadystatechange = delCb(xmlHttp, a);
    var b = new FormData();
    b.append("path", a);
    xmlHttp.open("DELETE", "/edit");
    xmlHttp.send(b);
}
function deleteRow(i) {
    IRjson.code.splice(i, 1);
    IRjson.name.splice(i, 1);
    makeIRList(IRjson);
    httpDelete("/IrRaw_Code" + i + ".txt");
}

function makeIRList(IRjson) {
    //document.getElementById("table").innerHTML = "<tbody id='body_table'><tbody>";
    var result = "",
        table_res = "";
    table_res = "<tr>" +
        "<td>номер</td>" +
        "<td>код</td>" +
        "<td>имя</td>" +
        // "<td>rawID</td>" +
        "<td>удалить</td>" +
        "</tr>";

    if (IRjson) {
        if (IRjson.code) {
            IRjson.name.splice(IRjson.code.length);
            for (i = 0; i < IRjson.code.length; i++) {
                IRjson.rawID[i] === undefined ? IRjson.rawID[i] = -1 : IRjson.rawID[i];
                table_res +=
                    "<tr id='number'>" +
                    "<td id='number'>" + i + "</td>" +
                    "<td id='code'>" + IRjson.code[i] + "</td>" +
                    "<td id='name'>" + IRjson.name[i] + "</td>" +
                    // "<td id='rawID'>" + IRjson.rawID[i] + "</td>" +
                    "<td id='del'><button class='form-control' onclick='deleteRow(" + i + ")'>X</button></td>" +
                    "</tr>";
            }
        }
    }

    //table_res="<tr><td></td><td><input type='text' id='IRcode' class='form-control'></td><td><input type='text' id='IRcodуName' class='form-control'></td><td></td></tr>";
    document.getElementById("table").innerHTML = table_res;

    //document.getElementById("codelist").innerHTML = result;
}
function WaitIR(submit) {
    progress = document.createElement("div");
    progress.id = "progress_IR";
    progress.className = "progress-bar progress-bar-striped active";
    //progress.innerHTML("class='progress-bar progress-bar-striped active'");
    my_div = document.getElementById("progress");
    my_div.appendChild(progress);
    progress.style.width = 100 + "%";
    progress.style.role = "progressbar";

    setVal("IRcode", "");
    server = "/WaitIR?IR='true'";
    old_submit = submit.value;
    readTextFile(server, function (data) {
        if (data === null) {
            progress.style.width = 0 + "%";
            //send_request(submit, server);
            take_progress_zero(submit);
        }
        else {
            sendReguestCode(data);
            progress.style.width = 0 + "%";
            take_progress_zero(submit);
        }
    });

    setTimeout(function () {
        take_progress_zero(submit);
        submit.value = old_submit;
        //submit_disabled(false);

    }, 5000);
    //send_request(submit, server);
    submit.value = 'нажмите кнопку...';
    //submit_disabled(true);
}
/*
 function send_request(submit, server) {
 request = new XMLHttpRequest();
 request.open("GET", server, true);
 request.send();
 request.addEventListener("IRRecieve", recieveIR, false);
 request.timeout = 10000; // time in milliseconds

 save_status(submit, request, server);

 }

 function save_status(submit, request, server) {
 old_submit = submit.value;
 request.onreadystatechange = function () {

 if (request.readyState === 4) {
 if (request.status === 200) {
 submit.value = "OK! взять IR";

 sendReguestCode(request.responseText);
 //var progress = document.getElementById("progress_IR");
 progress.style.width = 0 + "%";
 //progress.style.width = 100 + "%";
 //send_request(submit, server);
 take_progress_zero(submit);
 } else {
 progress.style.width = 0 + "%";
 //send_request(submit, server);
 take_progress_zero(submit);
 }
 }
 setTimeout(function () {
 take_progress_zero(submit);

 submit.value = old_submit;
 submit_disabled(false);
 //progress.remove();
 //progress.style.width=0+"%";
 //my_div.removeChild(progress);
 //progress.style.

 }, 5000);
 }
 request.ontimeout = function (e) {
 submit.value = old_submit;
 result_IR_prompt.value = "time end";
 take_progress_zero(submit);
 //save_status(submit, request);
 };
 submit.value = 'нажмите кнопку...';
 submit_disabled(true);
 }
 */
function take_progress_zero(submit) {
    submit.value = "Wait...";
    var progress = document.getElementById("progress_IR");
    progress.style.width = 0 + "%";

}

/*
 function readTextFile(file, callback) {
 //var rawFile = new XMLHttpRequest();
 var xmlHttp = createXmlHttpObject();
 xmlHttp.overrideMimeType("application/json");
 xmlHttp.open("GET", file, true);
 xmlHttp.timeout = 1000;
 xmlHttp.ontimeout = function () {
 callback({"response": 0});
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
function saveCommonCode(FileName, JsonFile) {
    saveData(FileName, JsonFile);

    /*
     JsonFile["fileName"] = FileName;
     try {
     var json_upload = "spec=0&fileName=" + FileName + "&json=" + JSON.stringify(JSON.parse(JsonFile));
     //setVal("test", json_upload);
     xmlHttp.open("POST", '/CommonSave', true);
     xmlHttp.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
     xmlHttp.send(json_upload);
     xmlHttp.onloadend = function () {
     //document.getElementById("test").innerHTML += JSON.stringify(IRjson);

     };
     if (xmlHttp.readyState === 4) {
     //saveCode();
     }
     }
     catch (e) {
     alert(e);
     }
     */
}
function AddNewButton() {

    var savedCode = getVal("IRcode");
    var NameIR = getVal("IRcodуName");
    if (IRjson===null){
        IRjson={code: [], name: [], rawID: [], rawCode: [], rawCodeLen: []};
    }
    if (savedCode.length > 30) {
        try {
            var Parsedata = JSON.parse(CODE);
            //setVal("test", Parsedata.raw);
            //Parsedata.c
            IRjson["rawCode"] == null ? IRjson["rawCode"] = [] : true;
            IRjson["rawCodeLen"] == null ? IRjson["rawCodeLen"] = [] : true;

            var idRaw = IRjson.name.length;
            if (idRaw !== -1) {
                IRjson["code"].push((idRaw));
                IRjson["rawID"].push((idRaw));
                IRjson["name"].push(NameIR);
                IRjson["rawCode"].push(Parsedata.c);
                IRjson["rawCodeLen"].push(Parsedata.len);
                saveCommonCode("IrRaw_Code" + idRaw + ".txt", CODE);
            }
            else {
            }
        } catch (e) {

        }
    }

    if ((savedCode.length < 20) && (savedCode.length > 1)) {
        IRjson.code.push(savedCode);
        IRjson.name.push(NameIR);

    }

    //IRjson=JSON.stringify(IRjson);
    makeIRList(IRjson);
    //document.getElementById("demo").innerHTML += JSON.stringify(IRjson);
}


function saveCode() {
    var SendCodeJson = {};

    SendCodeJson.name = IRjson.name;
    SendCodeJson.code = IRjson.code;
    SendCodeJson.num = IRjson.name.length;
    var json_upload = JSON.stringify(SendCodeJson);
    //json_upload.append("Number", btnId);
    setVal("test", json_upload);
    saveData("IRButtons.txt", SendCodeJson);

    /* xmlHttp.open("POST", '/SaveIR', true);
     xmlHttp.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');
     xmlHttp.send(json_upload);
     xmlHttp.onloadend = function () {
     //document.getElementById("test").innerHTML += JSON.stringify(IRjson);
     };
     */
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
function test() {
    sendReguestCode(JSON.stringify({
        "raw": "true",
        "len": 200,
        "c": [1, 2218, 2174, 266, 824, 280, 267, 280, 811, 279, 813, 277, 268, 266, 283, 278, 813, 264, 283, 280, 268, 277, 813, 266, 281, 264, 283, 266, 824, 280, 811, 279, 268, 280, 814, 278, 267, 266, 824, 266, 826, 264, 826, 280, 811, 266, 281, 280, 813, 277, 813, 279, 811, 280, 268, 279, 268, 280, 267, 280, 268, 279, 811, 266, 281, 280, 270, 280, 810, 278, 813, 277, 813, 266, 281, 266, 281, 280, 267, 266, 281, 266, 283, 277, 268, 280, 267, 280, 267, 266, 826, 264, 826, 266, 824, 280, 813, 278, 810, 280, 2601, 2190, 2192, 264, 826, 264, 283, 264, 825, 278, 815, 250, 295, 266, 283, 264, 826, 264, 286, 264, 281, 266, 838, 252, 281, 266, 283, 264, 826, 264, 826, 264, 283, 264, 829, 264, 281, 264, 826, 280, 813, 264, 826, 264, 827, 264, 283, 264, 826, 266, 840, 250, 826, 264, 283, 264, 283, 264, 283, 264, 283, 264, 826, 266, 294, 253, 283, 264, 826, 264, 840, 251, 826, 266, 281, 266, 294, 253, 283, 264, 281, 266, 283, 264, 283, 264, 283, 264, 283, 264, 826, 264, 826, 264, 826, 264, 828, 264, 826, 280, 0]
    }));
    // sendReguestCode("code");

}
function sendReguestCode(code) {
    CODE = code;
    setVal("Raw_code_checked", false);
    setVal("IRcode", code);
    try {
        var Parsedata = JSON.parse(code);
        if (Parsedata.c === undefined) {
            return
        }
        ;
        setVal("Raw_code_checked", Parsedata.raw);
        setVal("IRcode", Parsedata.c);
        setVal("test", Parsedata["c"]);
    } catch (e) {
        //document.getElementById("IRcode").value = code;
        setVal("test", e.toString());
    }

}

function handleMessage(data) {
    document.getElementById("IRcode").value = data;
}
/////////////////////////////helper