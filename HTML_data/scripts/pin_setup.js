const inputPinmode = [
    "no",
    "in",
    "out",
    "PWM",
    "ADC",
    "low. PWM",
    "DHT 1.1 Temp",
    "power MQ7",
    "DHT 1.1 Mist",
    "remote http",
    "power meter",
    "as5600",
    "MAC address",
    "EncA",
    "EncB",
    "ads1115",
    "ds18b20"
];

const inputWidget = ['unknown', 'switch', 'button', 'progress', 'progress-bar', 'chart', 'data'];
const availablePins = {
    "no": [255],
    "in": [5, 4, 14, 12, 13, 3, 17],
    "out": [16, 5, 4, 0, 2, 14, 12, 13, 15, 1],
    "PWM": [4, 5, 0, 2, 12, 14, 13, 15, 1],
    "ADC": [17],
    "low. PWM": [16, 5, 4, 0, 2, 14, 12, 13, 15, 1],
    "DHT 1.1 Temp": [5, 4, 14, 12, 13, 3, 17],
    "DHT 1.1 Mist": [5, 4, 14, 12, 13, 3, 17],
    "remote http": [255],
    "power meter": [255],
    "as5600": [5, 4, 14, 12, 13, 3, 17],
    "MAC address": [255],
    "EncA": [16, 5, 4, 0, 2, 14, 12, 13, 15, 1],
    "EncB": [16, 5, 4, 0, 2, 14, 12, 13, 15, 1],
    "ads1115": [16, 5, 4, 0, 2, 14, 12, 13, 15, 1],
    "ds18b20": [5, 4, 14, 12, 13, 3, 17, 2],
};

const pinInfo = {
    0: "D3, No interrupt, OK output. Connected to FLASH button, boot fails if pulled LOW. Pulled up.",
    1: "TX, TX pin, OK output. HIGH at boot, debug output at boot, boot fails if pulled LOW.",
    2: "D4, Pulled up, OK output. HIGH at boot, connected to on-board LED, boot fails if pulled LOW.",
    3: "RX, OK input, RX pin. HIGH at boot.",
    4: "D2, OK input, OK output. Often used as SDA (I2C).",
    5: "D1, OK input, OK output. Often used as SCL (I2C).",
    12: "D6, OK input, OK output. SPI (MISO).",
    13: "D7, OK input, OK output. SPI (MOSI).",
    14: "D5, OK input, OK output. SPI (SCLK).",
    15: "D8, Pulled to GND, OK output. SPI (CS), Boot fails if pulled HIGH.",
    16: "D0, No interrupt, No PWM or I2C support. HIGH at boot, used to wake up from deep sleep.",
    17: "A0, Analog Input, Not available for Digital I/O.",
    255: "there is no pin, guess that is -1"
};
const pinModeInfo = {
    "out": "you can write 0 or 1, in case of 0 - the output will not be inverted, in 1 - inverted",
    "in": "the input pin will be inverted, if it 0 - it will be triggered by gnd, if 1 - by 3.3V",
    "PWM": "in case if DefaultValue not 0, all value will be taken from actual value",
    "ADC": "will be ignorred",
    "ds18b20": "Default Value - will be number of connected items",
}
const modeValues = {
    "in": { min: 0, max: 1 },
    "out": { min: 0, max: 1 },
    "ADC": { min: 0, max: 1024 }
};

let tableData;

function fetchTableDataAndUpdateUI() {
    readTextFile("pin_setup.txt", function (result) {
        if (result !== 404) {
            tableData = JSON.parse(result);
        } else {
            tableData = {
                pinmode: [2, 2, 16],
                pin: [0, 16, 2],
                descr: ["roll_enable", "roll1_direction_0_down", "temp bedroom1"],
                widget: [1, 1, 5],
                IrBtnId: [255, 255, 255],
                defaultVal: [1, 1, 0]
            };
            console.error('Failed to fetch pin_setup.txt file. Using default values.');
        }
        renderTable();
    });
}

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
        //the code for a hint:
        pinSelect.addEventListener("mouseover", function (event) {
            if (pinInfo[event.target.value]) {
                const tooltip = document.createElement("div");
                tooltip.className = "tooltip";
                tooltip.textContent = pinInfo[event.target.value];
                tooltip.style.position = "absolute";
                tooltip.style.left = `${event.clientX + 10}px`;
                tooltip.style.top = `${event.clientY + 10}px`;
                document.body.appendChild(tooltip);
            }
        });
        pinSelect.addEventListener("mouseout", function () {
            const tooltips = document.getElementsByClassName("tooltip");
            while (tooltips.length > 0) {
                tooltips[0].parentNode.removeChild(tooltips[0]);
            }
        });
        //end hint
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
        const currentModeIndex = tableData.pinmode[index];
        const currentMode = inputPinmode[currentModeIndex];
        defaultInput.className = 'form-control';
        defaultInput.type = 'number';
        let minValue = -1;
        let maxValue = 1024;
        if (modeValues.hasOwnProperty(currentMode)) {
            maxValue = modeValues[currentMode].max;
            minValue = modeValues[currentMode].min;
        }
        defaultInput.min = minValue;
        defaultInput.max = maxValue;
        defaultInput.value = tableData.defaultVal[index];
        defaultInput.oninput = () => {
            tableData.defaultVal[index] = parseInt(defaultInput.value);
            saveDataToLocalStorage();
        };
        defaultInput.addEventListener("mouseover", function (event) {
            if (pinModeInfo[currentMode]) {
                const tooltip = document.createElement("div");
                tooltip.className = "tooltip";
                tooltip.textContent = pinModeInfo[currentMode];
                tooltip.style.position = "absolute";
                tooltip.style.left = `${event.pageX + 10}px`;
                tooltip.style.top = `${event.pageY + 10}px`;
                document.body.appendChild(tooltip);
            }
        });
        defaultInput.addEventListener("mouseout", function () {
            const tooltips = document.getElementsByClassName("tooltip");
            while (tooltips.length > 0) {
                tooltips[0].parentNode.removeChild(tooltips[0]);
            }
        });
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
    addRowButton.className = "btn btn-lg btn-primary btn-block";
    addRowButton.onclick = () => {
        if (tableData.pinmode.length < 10) {
            tableData.pinmode.push(0);
            tableData.pin.push(0);
            tableData.descr.push('');
            tableData.widget.push(0);
            tableData.IrBtnId.push(0);
            tableData.defaultVal.push(0);
        }
        saveDataToLocalStorage();
        renderTable();
    };
    container.appendChild(addRowButton);
    tableData.numberChosed = tableData.pinmode.length
}
function makeSave() {
    const jsonStr2 = JSON.stringify(tableData, null, 2);
    document.getElementById("output").appendChild(alert_message(jsonStr2));
    saveData("pin_setup.txt", jsonStr2, function (callback) {
        document.getElementById("output").appendChild(alert_message(callback));
    });
}
// renderTable();

fetchTableDataAndUpdateUI();

btnbtns = document.getElementById('btmBtns');
btnbtns.appendChild(bottomButtons2());
