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
        "scan": ["Fregat-74", "misha_home", "topol_3_1", "netis_F20B65", "Camel", "Home", "Evil", "TP-LINK_E1DEA0", "Link", "@HomeF328", "rockrobo-vacuum-v1_miap6DFF", "Vlados", "}|{eKa", "My network hamster", "kotunhome", "netis_91", "Tenda_2F4018", "Julia77"],
        enc: ["*", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""],
        RSSI: [-85, -60, -89, -94, -90, -29, -93, -89, -91, -92, -80, -91, -87, -88, -79, -89, -90, -87]
    };

    // Load WiFi list and process the callback
    readTextFile("wifiList", callbackWifiList);
    setHTML("btmBtns", bottomButtons());
    //send_request("wifiList", callbackWifiList);
    //callbackWifiList(JSON.stringify(data));
});

function callbackWifiList(response) {

    // Parse response and process WiFi list
    try {
        var data = JSON.parse(response);
        createBodyHead(data);
        tableCreate(data);
    } catch (e) {
        setHTML("input", e);
        //setHTML("output", response);
    }
}

function createBodyHead(data) {
    var body = document.getElementById('connectis');
    
    // Display the currently connected access point
    var newLi = document.createElement('li');
    newLi.className = "list-group-item";
    newLi.innerHTML = "You are connected to the access point: " + data.ssid;
    body.appendChild(newLi);
    
    // Display local IP
    var newLi = document.createElement('li');
    newLi.className = "list-group-item";
    newLi.innerHTML = "Local IP: " + data.WiFilocalIP;
    body.appendChild(newLi);
    
    // Display software access point name
    var newLi = document.createElement('li');
    newLi.className = "list-group-item";
    newLi.innerHTML = "Software access point: " + data.softAP_ssid;
    body.appendChild(newLi);
    
    // Display software access point IP
    var newLi = document.createElement('li');
    newLi.className = "list-group-item";
    newLi.innerHTML = "Software access point IP: " + data.WiFisoftAPIP;
    body.appendChild(newLi);
}

function tableCreate(data) {

    var body = document.getElementById('table_div');
    var tbl = document.createElement('table');
    tbl.className = "table";
    tbl.align = "center";
    tbl.setAttribute('border', '1');
    var tbdy = document.createElement('tbody');
    
    // Create table header
    var tr = document.createElement('tr');
    tbl.className = "table";

    createTD(tr, "WiFi SSID");
    createTD(tr, "Encryption");
    createTD(tr, "Signal Strength");
    tbdy.appendChild(tr);

    // Populate table with data from the scan results
    for (var i = 0; i < data.n; i++) {
        var tr = document.createElement('tr');
        var a = document.createElement('a');
        var linkText = document.createTextNode(data.scan[i]);
        a.appendChild(linkText);
        a.title = data.scan[i];
        a.href = "#";
        a.onclick = function () {
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
    body.appendChild(tbl);
}

// Helper function to create table cells
function createTD(tr, content) {
    var td = document.createElement('td');
    if (typeof content === "string" || typeof content === "number") {
        td.textContent = content;
    } else {
        td.appendChild(content);
    }
    tr.appendChild(td);
}