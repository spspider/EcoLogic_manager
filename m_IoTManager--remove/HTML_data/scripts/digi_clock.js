/**
 * Created by Tanja on 07.08.2019.
 */
document.addEventListener("DOMContentLoaded", function () {
    setHTML("btmBtns", bottomButtons());
    load();
});

function load() {
    setHTML("btmBtns", bottomButtons());
    readTextFile("digi_clock.txt", function (settings) {
        setHTML("input", getHTML("input") + settings);
        testJson(settings) ? loadBlock(settings) : null;

    });


}
function loadBlock(settings) {
    data2 = JSON.parse(settings);
    data = document.getElementsByTagName('body')[0].innerHTML;
    var new_string;
    for (var key in data2) {
        new_string = data.replace(new RegExp('{{' + key + '}}', 'g'), data2[key]);
        data = new_string;
    }
    document.getElementsByTagName('body')[0].innerHTML = new_string;
}
function toJSONString(form) {
    var obj = {};
    var elements = form.querySelectorAll("input, select, textarea");
    for (var i = 0; i < elements.length; ++i) {
        var element = elements[i];
        var name = element.name;
        var value = element.value;

        if (name) {
            obj[name] = value;
        }
    }

    return JSON.stringify(obj);
}
function CatchForm() {
    var form = document.getElementById("form");
    var output = document.getElementById("output");
    var JsonString = toJSONString(form);
    output.innerHTML = JsonString;
    saveData("digi_clock.txt", JsonString);

}
function SwitchOn() {
    var request = "/function?json={'WOL':'"+getVal("mac")+"'}";
    readTextFile(request, function (settings) {
        alert(settings);
    });
}