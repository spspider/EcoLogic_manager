document.addEventListener("DOMContentLoaded", load);

function load() {
    var g1 = [], g2 = [], g3 = [];
    var num = 88;
    var wh = 1;
    var FullLight = 255;
    var LowLight = 125;
    var color =255;
    var StartEndPosition=122;

    function binIndex(dec, maxSize) {
        var bin = (dec >>> 0).toString(2);
        var binchar = [];
        var i = 0;
        while (bin.length > 0) {
            var char = parseInt(bin.charAt(0)) === 1 ? LowLight : FullLight;
            bin = bin.slice(1);
            binchar[i] = char;
            i++;
        }
        while (i < maxSize) {
            binchar[i] = LowLight;
            i++;
        }
        return binchar;
    }

    var date = new Date();
    var hour = date.getHours();

    var concatBin;
    var dempfer = [];
    var i1 = 0;
    while (i1 < 22) {
        dempfer[i1] = FullLight;
        i1++;
    }
    concatBin = dempfer.concat(binIndex(hour, 5)).concat(binIndex(date.getMinutes(), 11)
        .concat(binIndex(date.getSeconds(), 6)));
    //console.log(concatBin);
    for (var i = 0; i < num; i++) {

        g1[i] = 255;
        g3[i] = concatBin[i] != null ? concatBin[i] : FullLight;
        //g3[i] = concatBin[i];
    }
    //console.log(g1.toString());
    var JsonString = {};
    JsonString.num = num;
    //JsonString.g1 = g1;
    JsonString.g1 = color;
    JsonString.g3 = g3;
    JsonString.wh = wh;

    var Stringify = JSON.stringify(JsonString);
    var length_str=Stringify.length/200;
    console.log(length_str);
    var link = "192.168.1.112/ws2811AJAXset?json=" + JSON.stringify(JsonString);
    //msg.url=link;
    //return msg;
    console.log(link);
    var button = "<a class='btn btn-block btn-default' href='" + link + "'>link</a>";
    setHTML("output", button);
}