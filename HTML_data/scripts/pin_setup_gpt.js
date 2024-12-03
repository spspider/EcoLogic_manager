const inputPinmode = ["no", "in", "out", "PWM", "ADC"];
const inputWidget = ['unknown', 'switch', 'button', 'progress', 'progress-bar', 'chart', 'data'];
const availablePins = {
    "in": [5, 4, 14, 12, 13],
    "out": [16, 5, 4, 0, 2, 14, 12, 13, 15, 3, 1],
    "PWM": [4, 5, 12, 14, 15],
    "ADC": [0],
};

const tableData = JSON.parse(localStorage.getItem('tableData')) || {
    pinmode: [2, 2, 16],
    pin: [0, 16, 2],
    descr: ["roll_enable", "roll1_direction_0_down", "temp bedroom1"],
    widget: [1, 1, 5],
    IrBtnId: [255, 255, 255],
    defaultVal: [1, 1, 0]
};

function saveDataToLocalStorage() {
    localStorage.setItem('tableData', JSON.stringify(tableData));
}

function renderTable() {
    const container = document.getElementById('sel1div');
    container.innerHTML = '';

    const table = document.createElement('table');
    table.className = 'table'

    // Header Row
    const headers = ['Pin Mode', 'Pin', 'Description', 'Widget', 'IR', 'Default', 'Actions'];
    const headerRow = document.createElement('tr');
    headers.forEach(header => {
        const th = document.createElement('th');
        th.textContent = header;
        headerRow.appendChild(th);
    });
    table.appendChild(headerRow);

    // Table Rows
    tableData.pinmode.forEach((_, index) => {
        const row = document.createElement('tr');

        // Pin Mode
        const pinModeCell = document.createElement('td');
        const pinModeSelect = document.createElement('select');
        pinModeSelect.className = 'form-control';
        inputPinmode.forEach((mode, idx) => {
            const option = document.createElement('option');
            option.value = idx;
            option.textContent = mode;
            if (idx === tableData.pinmode[index]) option.selected = true;
            pinModeSelect.appendChild(option);
        });
        pinModeSelect.onchange = () => {
            tableData.pinmode[index] = parseInt(pinModeSelect.value);
            renderTable();
            saveDataToLocalStorage();
        };
        pinModeCell.appendChild(pinModeSelect);
        row.appendChild(pinModeCell);

        // Pin
        const pinCell = document.createElement('td');
        const pinSelect = document.createElement('select');
        pinSelect.className = 'form-control';
        const availablePinsForMode = availablePins[inputPinmode[tableData.pinmode[index]]] || [];
        availablePinsForMode.forEach(pin => {
            const option = document.createElement('option');
            option.value = pin;
            option.textContent = pin;
            if (pin === tableData.pin[index]) option.selected = true;
            pinSelect.appendChild(option);
        });
        pinSelect.onchange = () => {
            tableData.pin[index] = parseInt(pinSelect.value);
            saveDataToLocalStorage();
        };
        pinCell.appendChild(pinSelect);
        row.appendChild(pinCell);

        // Description
        const descrCell = document.createElement('td');
        const descrInput = document.createElement('input');
        descrInput.type = 'text';
        descrInput.className = 'form-control';
        descrInput.value = tableData.descr[index];
        descrInput.oninput = () => {
            tableData.descr[index] = descrInput.value;
            saveDataToLocalStorage();
        };
        descrCell.appendChild(descrInput);
        row.appendChild(descrCell);

        // Widget
        const widgetCell = document.createElement('td');
        const widgetSelect = document.createElement('select');
        widgetSelect.className = 'form-control';
        inputWidget.forEach((widget, idx) => {
            const option = document.createElement('option');
            option.value = idx;
            option.textContent = widget;
            if (idx === tableData.widget[index]) option.selected = true;
            widgetSelect.appendChild(option);
        });
        widgetSelect.onchange = () => {
            tableData.widget[index] = parseInt(widgetSelect.value);
            saveDataToLocalStorage();
        };
        widgetCell.appendChild(widgetSelect);
        row.appendChild(widgetCell);

        // IR
        const irCell = document.createElement('td');
        const irInput = document.createElement('input');
        irInput.type = 'number';
        irInput.className = 'form-control';
        irInput.min = -1;
        irInput.max = 1024;
        irInput.value = tableData.IrBtnId[index];
        irInput.oninput = () => {
            tableData.IrBtnId[index] = parseInt(irInput.value);
            saveDataToLocalStorage();
        };
        irCell.appendChild(irInput);
        row.appendChild(irCell);

        // Default
        const defaultCell = document.createElement('td');
        const defaultInput = document.createElement('input');
        defaultInput.className = 'form-control';
        defaultInput.type = 'number';
        defaultInput.min = -1;
        defaultInput.max = 1024;
        defaultInput.value = tableData.defaultVal[index];
        defaultInput.oninput = () => {
            tableData.defaultVal[index] = parseInt(defaultInput.value);
            saveDataToLocalStorage();
        };
        defaultCell.appendChild(defaultInput);
        row.appendChild(defaultCell);

        // Actions
        const actionsCell = document.createElement('td');
        const deleteButton = document.createElement('button');
        deleteButton.className = "btn btn-primary btn-xs"
        deleteButton.textContent = 'Delete';
        deleteButton.onclick = () => {
            tableData.pinmode.splice(index, 1);
            tableData.pin.splice(index, 1);
            tableData.descr.splice(index, 1);
            tableData.widget.splice(index, 1);
            tableData.IrBtnId.splice(index, 1);
            tableData.defaultVal.splice(index, 1);
            saveDataToLocalStorage();
            renderTable();
        };
        actionsCell.appendChild(deleteButton);
        row.appendChild(actionsCell);

        table.appendChild(row);
    });

    container.appendChild(table);

    // Add Row Button
    const addRowButton = document.createElement('button');
    addRowButton.textContent = 'Add Row';
    addRowButton.onclick = () => {
        tableData.pinmode.push(0);
        tableData.pin.push(0);
        tableData.descr.push('');
        tableData.widget.push(0);
        tableData.IrBtnId.push(0);
        tableData.defaultVal.push(0);
        saveDataToLocalStorage();
        renderTable();
    };
    container.appendChild(addRowButton);

    // Save Button
    const saveButton = document.createElement('button');
    saveButton.textContent = 'Save';
    // saveButton.onclick = () => {
    //     const blob = new Blob([JSON.stringify(tableData, null, 2)], { type: 'application/json' });
    //     const a = document.createElement('a');
    //     a.href = URL.createObjectURL(blob);
    //     a.download = 'pinout.txt';
    //     a.click();
    // };
    // function makeSave() {
    //     const jsonStr2 = JSON.stringify(tableData, null, 2);
    //     saveData("pin_setup.txt", jsonStr2, function (callback) {
    //         document.getElementById("output").appendChild(alert_message(callback));
    //     });
    // }
    container.appendChild(saveButton);
}

renderTable();

btnbtns = document.getElementById('btmBtns');
btnbtns.appendChild(bottomButtons2());
