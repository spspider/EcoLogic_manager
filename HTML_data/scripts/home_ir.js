var DataChart = [];
var myChart = [];
var IR_station = {};
document.addEventListener('DOMContentLoaded', function () {
    if (typeof Chart !== 'undefined') {
        loadfields();
    }
});
function loadfields() {
    reloadPeriod = get_cookie("reloadPeriod");
    reloadPeriod = reloadPeriod < 500 ? 500 : reloadPeriod;
    setReloadPeriod();
    createCharts();
    loadValues();
}
function dataChartAdd(number, data) {
    var date = new Date();
    var time = date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds();
    DataChart[number].datasets[0].data.push(data);
    DataChart[number].labels.push(time);
    if (DataChart[number].datasets[0].data.length > 1000) {
        DataChart[number].datasets[0].data.splice(0, 1);
        DataChart[number].labels.splice(0, 1);
    }
    DataChart[number].datasets[0].label = labelChart[number] + ": " + data;
    myChart[number].update();
}
function loadValues() {
    //readTextFile("IR_station.txt", setValuesCallback);
}
function setValuesCallback(data) {
    setHTML("input", data);
    if (data === null) {
        clearMyTimeout();
        return;
    }
    ;
    IR_station = JSON.parse(data);
    setVal("mU", IR_station.mU);
    setVal("mB", IR_station.mB);
    setVal("tU", IR_station.tU);
    setVal("tB", IR_station.tB);
    dataChartAdd(0, IR_station.aTu);
    dataChartAdd(1, IR_station.aTb);
}
var running = false;
var reloadPeriod = 10000;
var timeOut;
var timeOut_answer;
function setReloadPeriod(thisItem) {
    reloadPeriod = (thisItem === undefined) ? reloadPeriod : thisItem.value;
    setVal("run_range", reloadPeriod);
    //reloadPeriod = thisItem.value;
    var refreshInput = document.getElementById("refresh-rate");
    set_cookie("reloadPeriod", reloadPeriod);
    refreshInput.value = reloadPeriod;
    refreshInput.onchange = function (e) {
        var value = parseInt(e.target.value);
        reloadPeriod = (value > 0) ? value : 0;
        e.target.value = reloadPeriod;
    }
    running = false;
    run();
}
function clearMyTimeout() {
    clearTimeout(timeOut); //останавливаем слудующий таймер, так как этот таймер не остановлен
    running = false;
    clearTimeout(timeOut_answer);
}
function run() {
    if (!running) {
        running = true;
        loadValuesRun_AJAX();
    }
}
function sendNewValue(thatVal) {
    clearMyTimeout();

    var sendJSON = JSON.stringify({
        't': thatVal.id,
        'v': thatVal.value
    });
    var callJson = "IR_Station" + "?json=" + sendJSON;
    readTextFile(callJson, setValuesCallback);
    setHTML("output", callJson);
    run();
}
function loadValuesRun_AJAX() {
    if (!running) return;
    var sendJSON = JSON.stringify({
        't': "status",
        'v': 0
    });
    if (running) {
//        readTextFile(("IRStation?json=" + sendJSON), setValuesCallback);
        readTextFile(("IR_Station.txt"), setValuesCallback);
        clearTimeout(timeOut);
        timeOut = setTimeout(loadValuesRun_AJAX, reloadPeriod);
        clearTimeout(timeOut_answer);
        timeOut_answer = setTimeout(clearMyTimeout, (reloadPeriod + 10000));//2 сек Для ответа, если ответа нет - соединение потеряно
    }
}
var labelChart = [];
function createCharts() {
    var contextChart = [];

    contextChart[0] = document.getElementById("chartUpper").getContext('2d');
    contextChart[1] = document.getElementById("chartBottom").getContext('2d');
    labelChart[0] = "верхний нагреватель";
    labelChart[1] = "нижний нагреватель";
    for (var i = 0; i < 2; i++) {
        DataChart[i] = {
            labels: [],
            ctx: contextChart[i],
            datasets: [{
                data: [],
                fill: false,
                label: labelChart[i],
                radius: 0,
                backgroundColor: "rgba(33, 170, 191,1)",
                borderColor: "rgba(33, 170, 191,1)"
            }]
        };
    }
    for (var i = 0; i < 2; i++) {
        var id = i;
        try {
            myChart[id] = new Chart(DataChart[id].ctx, {
                type: 'line',
                data: DataChart[id],
                options: {
                    tooltips: {
                        mode: 'index',
                        intersect: false
                    }
                }
            });
        } catch (e) {

        }
    }
}