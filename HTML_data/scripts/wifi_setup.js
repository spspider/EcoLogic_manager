/**
 * Created by sergey on 19.09.2017.
 */
document.addEventListener("DOMContentLoaded", function () {
    //load();
    var data = {
        ssid: "Home",
        softAP_ssid: "dev01-kitchen",
        "n": 10,
        WiFisoftAPIP: "0.0.0.0",
        ssid: "Home",
        WiFilocalIP: "192.168.1.108",
        "scan": ["Fregat-74", "misha_home", "topol_3_1", "netis_F20B65", "Camel", "Home", "Evil", "TP-LINK_E1DEA0", "Link", "@HomeF328", "rockrobo-vacuum-v1_miap6DFF", "Vlados", "}|{eKa", "My netwok hamster", "kotunhome", "netis_91", "Tenda_2F4018", "Julia77"],
        enc: ["*", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""],
        RSSI: [-85, -60, -89, -94, -90, -29, -93, -89, -91, -92, -80, -91, -87, -88, -79, -89, -90, -87]
    };

    readTextFile("wifiList", callbackWifiList);
    setHTML("btmBtns",bottomButtons());
    //send_request("wifiList", callbackWifiList);
    //callbackWifiList(JSON.stringify(data));
});
function callbackWifiList(respnose) {

    //data2 = JSON.parse(respnose);
    //setHTML("output", respnose);

    try {
        var data = JSON.parse(respnose);
        createBodyHead(data);
        tableCreate(data);
    }catch (e){
        setHTML("input",e);
        //setHTML("output",respnose);
    }


}
function createBodyHead(data) {
    var body = document.getElementById('connectis');
    var newLi = document.createElement('li');
    newLi.className = "list-group-item";
    newLi.innerHTML = "Вы подключены к точке досупа: " + data.ssid;
    body.appendChild(newLi);
    var newLi = document.createElement('li');
    newLi.className = "list-group-item";
    newLi.innerHTML = "Локальный IP:" + data.WiFilocalIP;
    body.appendChild(newLi);
    var newLi = document.createElement('li');
    newLi.className = "list-group-item";
    newLi.innerHTML = "Программная точка доступа: " + data.softAP;
    body.appendChild(newLi);
    var newLi = document.createElement('li');
    newLi.className = "list-group-item";
    newLi.innerHTML = "Программный IP точки доступа: " + data.WiFisoftAPIP;
    body.appendChild(newLi);


}

function tableCreate(data) {

    var body = document.getElementById('table_div');
    var tbl = document.createElement('table');
    tbl.className="table";
    tbl.align="center";
    //tbl.style.width = '100%';
    tbl.setAttribute('border', '1');
    var tbdy = document.createElement('tbody');
    var tr = document.createElement('tr');
    tbl.className = "table";
    var td = document.createElement('td');


    createTD(tr, "WiFi SSID");
    createTD(tr, "ENC");
    createTD(tr, "Level");
    tbdy.appendChild(tr);


    for (var i = 0; i < data.n; i++) {
        var tr = document.createElement('tr');
        //for (var j = 0; j < 3; j++) {
        var a = document.createElement('a');
        var linkText = document.createTextNode(data.scan[i]);
        a.appendChild(linkText);
        a.title = data.scan[i];
        a.href = "#";
        var text = data.enc[i];
        a.onclick = function (text) {
            setVal("ssid", this.title);
            document.getElementById('ssid').focus();
            document.getElementById('ssid').scrollIntoView();
        }
        createTD(tr, a);
        createTD(tr, data.enc[i]);
        createTD(tr, data.RSSI[i]);
        tbdy.appendChild(tr);
    }
    tbl.appendChild(tbdy);
    body.appendChild(tbl)
}