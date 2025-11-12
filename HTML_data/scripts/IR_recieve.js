document.addEventListener("DOMContentLoaded", load);

let IRjson = {code: [], name: [], rawID: [], rawCode: [], rawCodeLen: []};
let progress;
let CODE = {};

function saveToLocalStorage() {
    localStorage.setItem('IR_json', JSON.stringify(IRjson));
}

function loadFromLocalStorage() {
    const stored = localStorage.getItem('IR_json');
    if (stored) {
        try {
            IRjson = JSON.parse(stored);
            IRjson.rawID = IRjson.rawID || [];
            IRjson.code = IRjson.code || [];
            IRjson.name = IRjson.name || [];
            IRjson.rawCode = IRjson.rawCode || [];
            IRjson.rawCodeLen = IRjson.rawCodeLen || [];
        } catch (e) {
            IRjson = {code: [], name: [], rawID: [], rawCode: [], rawCodeLen: []};
        }
    }
}

function load() {
    document.getElementById("btmBtns").appendChild(bottomButtons());
    load2();
}

function load2() {
    readTextFile("IRButtons.txt", function (text) {
        if (!text) {
            loadFromLocalStorage();
            makeIRList(IRjson);
            return;
        }
        try {
            IRjson = JSON.parse(text);
            IRjson.rawID = IRjson.rawID || [];
            IRjson.code = IRjson.code || [];
            IRjson.name = IRjson.name || [];
            IRjson.rawCode = IRjson.rawCode || [];
            IRjson.rawCodeLen = IRjson.rawCodeLen || [];
            saveToLocalStorage();
            makeIRList(IRjson);
        } catch (e) {
            loadFromLocalStorage();
            makeIRList(IRjson);
        }
    });
}

function delCb(xmlHttp, a) {
    return function () {
        if (xmlHttp.readyState == 4) {
            if (xmlHttp.status != 200) {
                const testDiv = document.getElementById("test");
                if (testDiv) testDiv.appendChild(alert_message("Delete error", 3));
            }
        }
    }
}

function httpDelete(a) {
    const xmlHttp = createXmlHttpObject();
    xmlHttp.onreadystatechange = delCb(xmlHttp, a);
    const b = new FormData();
    b.append("path", a);
    xmlHttp.open("DELETE", "/edit");
    xmlHttp.send(b);
}

function dec2bin(dec){
    return (dec >>> 0).toString(2);
}

function sendCode(i) {
    setVal("bin", dec2bin(parseInt(i, 16)));
    readTextFile('/function?data={"sendIR":"' + i + '"}', function (callback) {
        const testDiv = document.getElementById("test");
        if (testDiv) {
            testDiv.appendChild(alert_message(callback, 3));
        }
    });
}

function sendCodeBin(){
    const hex = parseInt(getVal("bin"), 2).toString(16).toUpperCase();
    sendCode(hex);
}

function deleteRow(i) {
    IRjson.code.splice(i, 1);
    IRjson.name.splice(i, 1);
    IRjson.rawID.splice(i, 1);
    IRjson.rawCode.splice(i, 1);
    IRjson.rawCodeLen.splice(i, 1);
    saveToLocalStorage();
    makeIRList(IRjson);
    httpDelete("/IrRaw_Code" + i + ".txt");
}

function makeIRList(IRjson) {
    const table = document.getElementById("table");
    if (!table) return;
    
    table.innerHTML = '';
    
    const headerRow = document.createElement('tr');
    ['Number', 'Code', 'Name', 'Delete'].forEach(text => {
        const td = document.createElement('td');
        td.textContent = text;
        headerRow.appendChild(td);
    });
    table.appendChild(headerRow);
    
    if (IRjson && IRjson.code) {
        IRjson.name.splice(IRjson.code.length);
        for (let i = 0; i < IRjson.code.length; i++) {
            IRjson.rawID[i] = IRjson.rawID[i] !== undefined ? IRjson.rawID[i] : -1;
            
            const row = document.createElement('tr');
            
            const numTd = document.createElement('td');
            numTd.textContent = i;
            row.appendChild(numTd);
            
            const codeTd = document.createElement('td');
            const codeBtn = document.createElement('button');
            codeBtn.className = 'form-control';
            codeBtn.textContent = IRjson.code[i];
            codeBtn.onclick = () => sendCode(IRjson.code[i]);
            codeTd.appendChild(codeBtn);
            row.appendChild(codeTd);
            
            const nameTd = document.createElement('td');
            nameTd.textContent = IRjson.name[i];
            row.appendChild(nameTd);
            
            const delTd = document.createElement('td');
            const delBtn = document.createElement('button');
            delBtn.className = 'form-control';
            delBtn.textContent = 'X';
            delBtn.onclick = () => deleteRow(i);
            delTd.appendChild(delBtn);
            row.appendChild(delTd);
            
            table.appendChild(row);
        }
    }
}

function WaitIR(submit) {
    progress = document.createElement("div");
    progress.id = "progress_IR";
    progress.className = "progress-bar progress-bar-striped active";
    const my_div = document.getElementById("progress");
    my_div.appendChild(progress);
    progress.style.width = 100 + "%";
    progress.style.role = "progressbar";

    setVal("IRcode", "");
    const server = "/WaitIR?IR='true'";
    const old_submit = submit.value;
    readTextFile(server, function (data) {
        if (data === null) {
            progress.style.width = 0 + "%";
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
    }, 5000);
    submit.value = 'Press the button...';
}

function take_progress_zero(submit) {
    submit.value = "Wait...";
    const progress = document.getElementById("progress_IR");
    progress.style.width = 0 + "%";
}

function saveCommonCode(FileName, JsonFile) {
    saveData(FileName, JsonFile);
}

function AddNewButton() {
    if (!IRjson || typeof IRjson !== 'object') {
        IRjson = { code: [], name: [], rawID: [], rawCode: [], rawCodeLen: [] };
    }
    IRjson.code = IRjson.code || [];
    IRjson.name = IRjson.name || [];
    IRjson.rawID = IRjson.rawID || [];
    IRjson.rawCode = IRjson.rawCode || [];
    IRjson.rawCodeLen = IRjson.rawCodeLen || [];

    const savedCode = getVal("IRcode");
    const NameIR = getVal("IRcodName");

    if (savedCode.length > 30) {
        try {
            const Parsedata = JSON.parse(CODE);
            const idRaw = IRjson.name.length;
            if (idRaw !== -1) {
                IRjson.code.push(idRaw);
                IRjson.rawID.push(idRaw);
                IRjson.name.push(NameIR);
                IRjson.rawCode.push(Parsedata.c);
                IRjson.rawCodeLen.push(Parsedata.len);
                saveCommonCode("IrRaw_Code" + idRaw + ".txt", CODE);
            }
        } catch (e) {
            console.error('Parse error:', e);
        }
    }

    if ((savedCode.length < 20) && (savedCode.length > 1)) {
        IRjson.code.push(savedCode);
        IRjson.name.push(NameIR);
    }

    saveToLocalStorage();
    makeIRList(IRjson);
    
    const nameField = document.getElementById('IRcodName');
    if (nameField) nameField.value = '';
}

function saveCode() {
    const SendCodeJson = {};
    SendCodeJson.name = IRjson.name;
    SendCodeJson.code = IRjson.code;
    SendCodeJson.num = IRjson.name.length;
    const json_upload = JSON.stringify(SendCodeJson);
    setVal("test", json_upload);
    saveData("IRButtons.txt", SendCodeJson);
}

function saveData(filename, data) {
    const xmlHttp = createXmlHttpObject();
    const file = new Blob([JSON.stringify(data, null, 2)], {type: "text/plain;charset=utf-8"});
    const formData = new FormData();
    formData.append("data", file, filename);
    
    xmlHttp.onreadystatechange = function () {
        if (xmlHttp.readyState == 4) {
            const testDiv = document.getElementById("test");
            if (xmlHttp.status != 200) {
                if (testDiv) testDiv.appendChild(alert_message("ERROR[" + xmlHttp.status + "]: " + xmlHttp.responseText, 5));
            } else {
                if (testDiv) testDiv.appendChild(alert_message(xmlHttp.responseText, 3));
            }
        }
    };
    
    xmlHttp.open("POST", "/edit");
    xmlHttp.send(formData);
}

function test() {
    sendReguestCode(JSON.stringify({
        "raw": "true",
        "len": 200,
        "c": [1, 2218, 2174, 266, 824, 280, 267, 280, 811, 279, 813, 277, 268, 266, 283, 278, 813, 264, 283, 280, 268, 277, 813, 266, 281, 264, 283, 266, 824, 280, 811, 279, 268, 280, 814, 278, 267, 266, 824, 266, 826, 264, 826, 280, 811, 266, 281, 280, 813, 277, 813, 279, 811, 280, 268, 279, 268, 280, 267, 280, 268, 279, 811, 266, 281, 280, 270, 280, 810, 278, 813, 277, 813, 266, 281, 266, 281, 280, 267, 266, 281, 266, 283, 277, 268, 280, 267, 280, 267, 266, 826, 264, 826, 266, 824, 280, 813, 278, 810, 280, 2601, 2190, 2192, 264, 826, 264, 283, 264, 825, 278, 815, 250, 295, 266, 283, 264, 826, 264, 286, 264, 281, 266, 838, 252, 281, 266, 283, 264, 826, 264, 826, 264, 283, 264, 829, 264, 281, 264, 826, 280, 813, 264, 826, 264, 827, 264, 283, 264, 826, 266, 840, 250, 826, 264, 283, 264, 283, 264, 283, 264, 283, 264, 826, 266, 294, 253, 283, 264, 826, 264, 840, 251, 826, 266, 281, 266, 294, 253, 283, 264, 281, 266, 283, 264, 283, 264, 283, 264, 283, 264, 826, 264, 826, 264, 826, 264, 828, 264, 826, 280, 0]
    }));
}

function sendReguestCode(code) {
    CODE = code;
    setVal("Raw_code_checked", false);
    setVal("IRcode", code);
    try {
        const Parsedata = JSON.parse(code);
        if (Parsedata.c === undefined) {
            return;
        }
        setVal("Raw_code_checked", Parsedata.raw);
        setVal("IRcode", Parsedata.c);
        setVal("test", Parsedata["c"]);
    } catch (e) {
        setVal("test", e.toString());
    }
}

function handleMessage(data) {
    document.getElementById("IRcode").value = data;
}
