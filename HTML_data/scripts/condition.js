// Unified Condition Manager - Works on ESP8266 and Server
const IS_SERVER = window.location.pathname.startsWith('/api/');
const API_PREFIX = IS_SERVER ? '/api' : '';

document.addEventListener("DOMContentLoaded", init);

let tableData = [];
let pinSetup = {};
let id = 0;

const CONDITION_TYPES = ["none", "on time reached", "equal", "greater than", "less than", "timer"];
const ACTION_TYPES = ["no", "switch pin", "switch button", "switch remote button", "send Email", 
                      "switch condition", "switch mqtt request", "turn on 8211 strip", "WOL", 
                      "set timer", "move slider"];
const PIN_VALUES = ["0", "1", "PWM"];

function getDeviceId() {
    const params = new URLSearchParams(window.location.search);
    return params.get('device_id') || window.DEVICE_ID || 'default';
}

function init() {
    id = getParameterByName('id') || 0;
    loadPinSetup();
    document.getElementById("btmBtns").appendChild(bottomButtons());
}

function getParameterByName(name, url) {
    if (!url) url = window.location.href;
    name = name.replace(/[\[\]]/g, "\\$&");
    const regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)");
    const results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, " "));
}

function loadPinSetup() {
    if (IS_SERVER) {
        const deviceId = getDeviceId();
        fetch(`/api/pin_setup?device_id=${deviceId}`)
            .then(res => res.json())
            .then(data => {
                pinSetup = data;
                makeSelect();
                loadCondition();
            })
            .catch(err => console.error("Failed to load pin_setup", err));
    } else {
        readTextFile("pin_setup.txt", function (text) {
            if (text) {
                try {
                    pinSetup = JSON.parse(text);
                    makeSelect();
                    loadCondition();
                } catch (e) {
                    console.error("Failed to parse pin_setup", e);
                }
            }
        });
    }
}

function makeSelect() {
    let options = "";
    for (let i = 0; i < (pinSetup.numberChosed || 3); i++) {
        options += `<option>№:${i} ${pinSetup.descr[i] || ''}</option>`;
    }
    setHTML("select_condition", `<select class='form-control' id='NumberWidget' onchange='loadCondition()'>${options}</select>`);
    if (document.getElementById("NumberWidget")) {
        document.getElementById("NumberWidget").selectedIndex = id;
    }
}

function loadCondition() {
    const widgetId = document.getElementById("NumberWidget") ? document.getElementById("NumberWidget").selectedIndex : 0;
    id = widgetId;
    
    if (IS_SERVER) {
        const deviceId = getDeviceId();
        fetch(`/api/conditions?device_id=${deviceId}`)
            .then(res => res.json())
            .then(data => {
                tableData = data.length > 0 ? data : [];
                renderTable();
            })
            .catch(() => {
                const stored = localStorage.getItem('conditionData_' + id);
                tableData = stored ? JSON.parse(stored) : [];
                renderTable();
            });
    } else {
        readTextFile(`Condition${id}.txt`, function (text) {
            if (text) {
                try {
                    const data = JSON.parse(text);
                    convertFromStorage(data);
                    renderTable();
                } catch (e) {
                    tableData = [];
                    renderTable();
                }
            } else {
                tableData = [];
                renderTable();
            }
        });
    }
}

function convertFromStorage(data) {
    tableData = [];
    if (!data.tID) return;
    
    for (let i = 0; i < data.tID.length; i++) {
        const condType = CONDITION_TYPES[data.type[i]] || CONDITION_TYPES[0];
        let condValue = data.type_value[i] || "";
        
        if (condType === "on time reached") {
            condValue = minutesToTime(condValue);
        }
        
        tableData.push({
            enabled: data.En[i] === 1,
            sourcePin: "",
            conditionType: condType,
            conditionValue: condValue,
            actionType: ACTION_TYPES[data.act[i]] || ACTION_TYPES[0],
            actionValue: "",
            actionPin: "",
            pwmValue: "",
            actOnRaw: data.actOn[i]
        });
        
        parseActOn(i, data.act[i], data.actOn[i]);
    }
}

function parseActOn(index, actType, actOn) {
    const parts = String(actOn).split(' ');
    
    if (actType === 1) { // switch pin
        tableData[index].actionPin = parts[0];
        if (parts[1] == 2) {
            tableData[index].actionValue = "PWM";
            tableData[index].pwmValue = parts[2] || "";
        } else {
            tableData[index].actionValue = parts[1] === "1" ? "1" : "0";
        }
    } else if (actType === 2) { // switch button
        tableData[index].actionPin = parts[0];
        if (parts[1] == 2) {
            tableData[index].actionValue = "PWM";
            tableData[index].pwmValue = parts[2] || "";
        } else {
            tableData[index].actionValue = parts[1] === "1" ? "1" : "0";
        }
    }
}

function minutesToTime(minutes) {
    const hours = Math.floor(minutes / 60);
    const mins = Math.floor(minutes % 60);
    return `${String(hours).padStart(2, '0')}:${String(mins).padStart(2, '0')}`;
}

function timeToMinutes(time) {
    const [hours, minutes] = time.split(":");
    return parseInt(hours) * 60 + parseInt(minutes);
}

function saveDataToLocalStorage() {
    localStorage.setItem('conditionData_' + id, JSON.stringify(tableData));
}

function addCondition() {
    tableData.push({
        enabled: true,
        sourcePin: "",
        conditionType: CONDITION_TYPES[0],
        conditionValue: "",
        actionType: ACTION_TYPES[0],
        actionValue: "",
        actionPin: "",
        pwmValue: ""
    });
    saveDataToLocalStorage();
    renderTable();
}

function deleteCondition(index) {
    tableData.splice(index, 1);
    saveDataToLocalStorage();
    renderTable();
}

function renderTable() {
    const table = document.getElementById("table");
    table.innerHTML = "";
    
    tableData.forEach((row, index) => {
        const tr = document.createElement("tr");
        
        // Enabled checkbox
        const tdEnabled = document.createElement("td");
        const checkbox = document.createElement("input");
        checkbox.type = "checkbox";
        checkbox.checked = row.enabled;
        checkbox.onchange = () => updateRow(index, 'enabled', checkbox.checked);
        tdEnabled.appendChild(document.createTextNode(index + " "));
        tdEnabled.appendChild(checkbox);
        tr.appendChild(tdEnabled);
        
        // Source pin selector
        const tdSource = document.createElement("td");
        const sourceSelect = createSourceSelect(row.sourcePin, 
            (val) => updateRow(index, 'sourcePin', val));
        tdSource.appendChild(sourceSelect);
        tr.appendChild(tdSource);
        
        // Condition type
        const tdCondType = document.createElement("td");
        const selectCond = createSelect(CONDITION_TYPES, row.conditionType, 
            (val) => updateRow(index, 'conditionType', val));
        tdCondType.appendChild(selectCond);
        tr.appendChild(tdCondType);
        
        // Condition value
        const tdCondValue = document.createElement("td");
        const condInput = createConditionInput(row.conditionType, row.conditionValue, 
            (val) => updateRow(index, 'conditionValue', val));
        tdCondValue.appendChild(condInput);
        tr.appendChild(tdCondValue);
        
        // Action type
        const tdActType = document.createElement("td");
        const selectAct = createSelect(ACTION_TYPES, row.actionType, 
            (val) => updateRow(index, 'actionType', val));
        tdActType.appendChild(selectAct);
        tr.appendChild(tdActType);
        
        // Action details
        const tdActDetails = document.createElement("td");
        const actDetails = createActionDetails(row, index);
        tdActDetails.appendChild(actDetails);
        tr.appendChild(tdActDetails);
        
        // Delete button
        const tdDelete = document.createElement("td");
        const btnDelete = document.createElement("button");
        btnDelete.className = "form-control";
        btnDelete.textContent = "X";
        btnDelete.onclick = () => deleteCondition(index);
        tdDelete.appendChild(btnDelete);
        tr.appendChild(tdDelete);
        
        table.appendChild(tr);
    });
    
    updateButtons();
}

function createSourceSelect(selected, onChange) {
    const select = document.createElement("select");
    select.className = "form-control";
    
    const noneOpt = document.createElement("option");
    noneOpt.value = "";
    noneOpt.textContent = "-- select --";
    select.appendChild(noneOpt);
    
    if (pinSetup.descr) {
        const pinmodeNames = ["none", "input", "output", "PWM", "ADC", "DHT temp", "DHT hum", "DS18B20", "power meter"];
        pinSetup.descr.forEach((desc, i) => {
            const option = document.createElement("option");
            option.value = i;
            const pinmodeName = pinmodeNames[pinSetup.pinmode[i]] || "unknown";
            option.textContent = `${desc} (${pinmodeName})`;
            if (i == selected) option.selected = true;
            select.appendChild(option);
        });
    }
    
    select.onchange = () => onChange(select.value);
    return select;
}

function createSelect(options, selected, onChange) {
    const select = document.createElement("select");
    select.className = "form-control";
    
    options.forEach(opt => {
        const option = document.createElement("option");
        option.value = opt;
        option.textContent = opt;
        if (opt === selected) option.selected = true;
        select.appendChild(option);
    });
    
    select.onchange = () => onChange(select.value);
    return select;
}

function createConditionInput(type, value, onChange) {
    const container = document.createElement("div");
    
    if (type === "on time reached") {
        const input = document.createElement("input");
        input.type = "time";
        input.className = "form-control";
        input.value = value || "00:00";
        input.onchange = () => onChange(input.value);
        container.appendChild(input);
    } else if (type === "timer") {
        const input = document.createElement("input");
        input.type = "number";
        input.className = "form-control";
        input.value = value || "0";
        input.placeholder = "seconds";
        input.onchange = () => onChange(input.value);
        container.appendChild(input);
    } else if (type !== "none") {
        const input = document.createElement("input");
        input.type = "text";
        input.className = "form-control";
        input.value = value || "";
        input.onchange = () => onChange(input.value);
        container.appendChild(input);
    } else {
        container.textContent = "wait";
    }
    
    return container;
}

function createActionDetails(row, index) {
    const container = document.createElement("div");
    
    if (row.actionType === "switch pin") {
        const pinSelect = document.createElement("select");
        pinSelect.className = "form-control";
        [0, 2, 4, 5, 10, 12, 13, 14, 15, 16, 17].forEach(pin => {
            const opt = document.createElement("option");
            opt.value = pin;
            opt.textContent = pin;
            if (pin == row.actionPin) opt.selected = true;
            pinSelect.appendChild(opt);
        });
        pinSelect.onchange = () => updateRow(index, 'actionPin', pinSelect.value);
        container.appendChild(pinSelect);
        
        const valSelect = createSelect(PIN_VALUES, row.actionValue, 
            (val) => updateRow(index, 'actionValue', val));
        container.appendChild(valSelect);
        
        if (row.actionValue === "PWM") {
            const pwmInput = document.createElement("input");
            pwmInput.type = "number";
            pwmInput.className = "form-control";
            pwmInput.placeholder = "PWM value";
            pwmInput.value = row.pwmValue || "";
            pwmInput.onchange = () => updateRow(index, 'pwmValue', pwmInput.value);
            container.appendChild(pwmInput);
        }
    } else if (row.actionType === "switch button") {
        const btnSelect = document.createElement("select");
        btnSelect.className = "form-control";
        
        if (pinSetup.descr) {
            pinSetup.descr.forEach((desc, i) => {
                const opt = document.createElement("option");
                opt.value = i;
                opt.textContent = desc;
                if (i == row.actionPin) opt.selected = true;
                btnSelect.appendChild(opt);
            });
        }
        
        btnSelect.onchange = () => updateRow(index, 'actionPin', btnSelect.value);
        container.appendChild(btnSelect);
        
        const valSelect = createSelect(PIN_VALUES, row.actionValue, 
            (val) => updateRow(index, 'actionValue', val));
        container.appendChild(valSelect);
        
        if (row.actionValue === "PWM") {
            const pwmInput = document.createElement("input");
            pwmInput.type = "number";
            pwmInput.className = "form-control";
            pwmInput.value = row.pwmValue || "";
            pwmInput.onchange = () => updateRow(index, 'pwmValue', pwmInput.value);
            container.appendChild(pwmInput);
        }
    } else {
        container.textContent = "wait";
    }
    
    return container;
}

function updateRow(index, field, value) {
    tableData[index][field] = value;
    saveDataToLocalStorage();
    renderTable();
}

function updateButtons() {
    const btnContainer = document.getElementById("firstButton");
    btnContainer.innerHTML = "";
    
    const addBtn = document.createElement("input");
    addBtn.type = "submit";
    addBtn.className = "btn btn-lg btn-primary btn-block";
    addBtn.value = `Add condition: ${tableData.length}`;
    addBtn.onclick = addCondition;
    btnContainer.appendChild(addBtn);
    
    const saveBtn = document.createElement("input");
    saveBtn.type = "submit";
    saveBtn.className = "btn btn-lg btn-success btn-block";
    saveBtn.value = "Save";
    saveBtn.onclick = saveToDevice;
    btnContainer.appendChild(saveBtn);
}

function saveToDevice() {
    if (IS_SERVER) {
        const deviceId = getDeviceId();
        fetch(`/api/conditions?device_id=${deviceId}`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(tableData)
        })
        .then(res => res.json())
        .then(() => {
            const msg = alert_message('Conditions saved successfully!', 3);
            document.body.appendChild(msg);
        })
        .catch(err => {
            const msg = alert_message('Error saving: ' + err, 3);
            document.body.appendChild(msg);
        });
    } else {
        const jsonData = {
            En: [],
            tID: [],
            type: [],
            type_value: [],
            act: [],
            actOn: [],
            ID: id,
            Numbers: tableData.length
        };
        
        tableData.forEach((row, i) => {
            jsonData.En.push(row.enabled ? 1 : 0);
            jsonData.tID.push(i);
            jsonData.type.push(CONDITION_TYPES.indexOf(row.conditionType));
            
            let typeValue = parseInt(row.conditionValue) || 0;
            if (row.conditionType === "on time reached") {
                typeValue = timeToMinutes(row.conditionValue);
            }
            jsonData.type_value.push(typeValue);
            
            jsonData.act.push(ACTION_TYPES.indexOf(row.actionType));
            
            let actOn = "";
            const actType = ACTION_TYPES.indexOf(row.actionType);
            if (actType === 1 || actType === 2) {
                if (row.actionValue === "PWM") {
                    actOn = `${row.actionPin} 2 ${row.pwmValue}`;
                } else {
                    actOn = `${row.actionPin} ${row.actionValue}`;
                }
            }
            jsonData.actOn.push(actOn);
        });
        
        setHTML("output", JSON.stringify(jsonData));
        saveData(`Condition${id}.txt`, jsonData, false);
    }
}
