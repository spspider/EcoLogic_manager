// Condition Manager - Modern Implementation
document.addEventListener("DOMContentLoaded", init);

let tableData = [];
let pinSetup = {};

const CONDITION_TYPES = ["none", "on time reached", "equal", "greater than", "less than", "timer"];
const ACTION_TYPES = ["no", "switch pin", "switch button", "switch remote button", "send Email", 
                      "switch condition", "switch mqtt request", "turn on 8211 strip", "WOL", 
                      "set timer", "move slider"];
const PIN_VALUES = ["0", "1", "PWM"];

function init() {
    loadDataFromServer();
    setHTML("btmBtns", bottomButtons());
    
    const addBtn = document.querySelector("#firstButton input[value*='add condition']");
    if (addBtn) {
        addBtn.onclick = addCondition;
    }
    
    const saveBtn = document.createElement('input');
    saveBtn.type = 'submit';
    saveBtn.className = 'btn btn-lg btn-success btn-block';
    saveBtn.value = 'Save to Server';
    saveBtn.onclick = saveToServer;
    document.getElementById('firstButton').appendChild(saveBtn);
}

function loadDataFromServer() {
    const deviceId = window.DEVICE_ID || 'default';
    
    Promise.all([
        fetch(`/api/pin_setup?device_id=${deviceId}`).then(res => res.json()),
        fetch(`/api/conditions?device_id=${deviceId}`).then(res => res.json())
    ])
    .then(([pinData, condData]) => {
        pinSetup = pinData;
        tableData = condData.length > 0 ? condData : [];
        renderTable();
    })
    .catch(() => {
        loadConditionsFromLocalStorage();
        renderTable();
    });
}

function loadConditionsFromLocalStorage() {
    const stored = localStorage.getItem('tableData');
    if (stored) {
        tableData = JSON.parse(stored);
    }
}

function saveDataToLocalStorage() {
    localStorage.setItem('tableData', JSON.stringify(tableData));
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
        
        // Source pin/button selector
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
}

function createSourceSelect(selected, onChange) {
    const select = document.createElement("select");
    select.className = "form-control";
    
    const noneOpt = document.createElement("option");
    noneOpt.value = "";
    noneOpt.textContent = "-- select --";
    select.appendChild(noneOpt);
    
    if (pinSetup.descr) {
        pinSetup.descr.forEach((desc, i) => {
            const option = document.createElement("option");
            option.value = i;
            const pinmodeNames = ["none", "input", "output", "PWM", "ADC", "DHT temp", "DHT hum", "DS18B20", "power meter"];
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
        // Pin selector
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
        
        // Value selector
        const valSelect = createSelect(PIN_VALUES, row.actionValue, 
            (val) => updateRow(index, 'actionValue', val));
        container.appendChild(valSelect);
        
        // PWM input if needed
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

function saveToServer() {
    const deviceId = window.DEVICE_ID || 'default';
    
    fetch(`/api/conditions?device_id=${deviceId}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(tableData)
    })
    .then(res => res.json())
    .then(() => {
        const msg = document.createElement('div');
        msg.className = 'alert alert-success';
        msg.textContent = 'Conditions saved successfully!';
        document.body.appendChild(msg);
        setTimeout(() => msg.remove(), 3000);
    })
    .catch(err => {
        const msg = document.createElement('div');
        msg.className = 'alert alert-danger';
        msg.textContent = 'Error saving: ' + err;
        document.body.appendChild(msg);
        setTimeout(() => msg.remove(), 3000);
    });
}
