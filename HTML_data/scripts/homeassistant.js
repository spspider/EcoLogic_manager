/*


Do not alter this comments:
create generator for homeassistant, which will take values from pin_setup.txt:

{"numberChosed":1,"pinmode":[2,2,2,1,1,3,3,3,1,16],"pin":[0,0,1,0,0,0,0,0,0,0],"descr":["relay1","relay2","relay3","input1","input2","pwm1","pwm2","pwm3","input3","temp"],"widget":[1,1,1,6,6,3,3,3,6,5],"IrBtnId":[255,255,255,0,0,0,0,0,0,0],"defaultVal":[1,1,1,0,0,0,0,0,0,0]}

and will create homeassistant RESTful code, 

lets look deeply in pinmode, for each number there is a each RESTful config where
pinmode[] = 2 - is a switch, only 0 or 1
pinmode[] = 1 is a sensor, only output available
pinmode[] = 6 is a sensor, temperature only output available
pinmode[] = 8 is a sensor, humidity only output available
pinmode[] = 10 is a sensor power meter for electricity, only output avialable
pinmode[] = 3 is a range, where range is between 0 and 1024
pinmode[] = 16 same as sensor, only output

widget[] = 1 is a standard switcher
widget[] = 6 is a standard sensor output
widget[] = 3 is a standard range
widget[] = 5 is a chart sensor output
widget[] = 2 is a button

defaultVal[] = 0, it means, if you want to set relay to on, you have to pass "1", if off - pass "0"
defaultVal[] = 1, it means, if you want to set relay to on, you have to pass "0", if off - pass "1"


code should create something like this:

  backyard_light_on:
    url: "http://192.168.1.150/sendAJAX?json={\"t\":2,\"v\":0}" where "t" is array number, and "v" is value
    method: GET - always will be GET request
  backyard_light_off:
    url: "http://192.168.1.150/sendAJAX?json={\"t\":2,\"v\":1}"
    method: GET

IP address 192.168.1.150 - should be taken from device address using javscript
where "backyard_light_on" is taken from pin_setup.txt, in my example that is "relay1" (this name taken from "descr" array)

also it should generate template

  - platform: template
    switches:
      backyard_light:
        friendly_name: "Backyard Light"
        value_template: "{{ False }}"
        turn_on:
          service: rest_command.backyard_light_on
        turn_off:
          service: rest_command.backyard_light_off
        icon_template: mdi:lightbulb-outline
        unique_id: "backyard_light"


for the sensor value it should generate sensor like this:

rest:
  # Temperature Sensor
  - resource: "http://192.168.1.150/sendAJAX?json=%7B%22t%22:5,%22v%22:0%7D"
    scan_interval: 60 # hardcoded
    method: GET #hardcoded
    sensor:
      - name: "ESP8266 Temperature" - you can take name from "descr"
        unique_id: esp8266_temperature - also from "descr"
        state_class: measurement
        unit_of_measurement: "°C"  # dont need to add this, if it not temperature
        # Parse the return value
        value_template: "{{ value_json | float(default=0) }}" # return is a just text


for the slider (range) it should generate output like this:

"""
rest_command:
  set_nightlight:
    url: "http://192.168.1.150/sendAJAX?json={\"t\":3,\"v\":{{ value }}}" same IP address from URL, 3 - that is array number, 
    method: GET

input_number:
  nightlight_slider:
    name: NightLight Slider
    min: 0
    max: 1024
    step: 1

automation:
  - alias: "Set NightLight Value"
    trigger:
      platform: state
      entity_id: input_number.nightlight_slider
    action:
      - service: rest_command.set_nightlight
        data:
          value: "{{ states('input_number.nightlight_slider') }}"
"""

this output in yaml structured lines, should be attached to  <p id="bodyNode"></p> in main html page


the start code is:
"""

document.addEventListener("DOMContentLoaded", loadPinSetup);
var PinSetup = {};


function loadPinSetup() {
    setHTML("btmBtns", bottomButtons());
    readTextFile("pin_setup.txt", PinSetupLoaded);
}
function PinSetupLoaded(data) {
    if (data == null) {
        setHTML("bodyNode", "не удалось загрузить настройки пинов");
        return;
    }
    PinSetup = JSON.parse(data);

    setHTML("bodyNode", JSON.stringify(PinSetup));
}



"""



*/

document.addEventListener("DOMContentLoaded", firstload);
var PinSetup = {};

async function loadFileAsync(filePath) {
  return new Promise((resolve) => {
    readTextFile(filePath, (text) => resolve(text || null));
  });
}

async function firstload() {
  setHTML("btmBtns", bottomButtons());
  // readTextFile("pin_setup.txt", PinSetupLoaded);
  PinSetup = await loadFileAsync("pin_setup.txt");
  if (PinSetup) {
    PinSetupLoaded(PinSetup)
    generateHomeAssistantConfig()
    // Other_setup = await loadFileAsync("other_setup.txt");
    // console.log(JSON.stringify(Other_setup))
  }

}
function PinSetupLoaded(data) {
  if (data == null) {
    setHTML("bodyNode", "не удалось загрузить настройки пинов");
    return;
  }
  PinSetup = JSON.parse(data);
}

function generateHomeAssistantConfig() {
  let config = '';
  const ip = window.location.host; // Assuming IP is static for demonstration

  PinSetup.pinmode.forEach((mode, index) => {
    const pin = PinSetup.pin[index];
    const description = PinSetup.descr[index];
    const widget = PinSetup.widget[index];
    const defaultVal = PinSetup.defaultVal[index];

    switch (mode) {
      case 2: // Switch
        config += generateSwitchConfig(description, index, defaultVal, ip);
        break;
      case 1: // Sensor
      case 16: // Sensor
        config += generateSensorConfig(description, index, ip);
        break;
      case 6: // Temperature Sensor
        config += generateTemperatureSensorConfig(description, index, ip);
        break;
      case 7: // Humidity Sensor
        config += generateHumiditySensorConfig(description, index, ip);
        break;
      case 3: // Range
        config += generateRangeConfig(description, index, ip);
        break;
    }

  });
  config += generateFullSensorConfig(ip);
  setHTML("bodyNode", config);
}

function generateSwitchConfig(description, index, defaultVal, ip) {
  const onValue = defaultVal === 1 ? 0 : 1;
  const offValue = defaultVal === 1 ? 1 : 0;
  return `
rest_command:
  ${description}_on:
    url: "http://${ip}/sendAJAX?json={\\"t\\":${index},\\"v\\":${onValue}}"
    method: GET
  ${description}_off:
    url: "http://${ip}/sendAJAX?json={\\"t\\":${index},\\"v\\":${offValue}}"
    method: GET

- platform: template
  switches:
    ${description}:
      friendly_name: "${description.replace(/_/g, ' ')}"
      value_template: "{{ False }}"
      turn_on:
        service: rest_command.${description}_on
      turn_off:
        service: rest_command.${description}_off
      icon_template: mdi:lightbulb-outline
      unique_id: "${description}"
`;
}

function generateSensorConfig(description, index, ip) {
  return `
rest:
  - resource: "http://${ip}/sendAJAX?json=%7B%22t%22:${index},%22v%22:0%7D"
    scan_interval: 60
    method: GET
    sensor:
      - name: "${description}"
        unique_id: ${description.toLowerCase().replace(/\s+/g, '_')}
        state_class: measurement
        value_template: "{{ value_json | float(default=0) }}"
`;
}

function generateHumiditySensorConfig(description, index, ip) {
  return `
rest:
  - resource: "http://${ip}/sendAJAX?json=%7B%22t%22:${index},%22v%22:0%7D"
    scan_interval: 60
    method: GET
    sensor:
      - name: "${description}"
        unique_id: ${description.toLowerCase().replace(/\s+/g, '_')}
        state_class: measurement
        unit_of_measurement: "%"
        value_template: "{{ value_json | float(default=0) }}"
`;
}
function generateRangeConfig(description, index, ip) {
  return `
rest_command:
  set_${description.toLowerCase()}:
    url: "http://${ip}/sendAJAX?json={\\"t\\":${index},\\"v\\":{{ value }}}"
    method: GET

input_number:
  ${description}_slider:
    name: ${description} Slider
    min: 0
    max: 1024
    step: 1

automation:
  - alias: "Set ${description} Value"
    trigger:
      platform: state
      entity_id: input_number.${description}_slider
    action:
      - service: rest_command.set_${description.toLowerCase()}
        data:
          value: "{{ states('input_number.${description}_slider') }}"
`;
}

function generateFullSensorConfig(ip) {
  return `
rest:
- resource: "http://${ip}/sendAJAX?json=%7B%22t%22:127,%22v%22:0%7D"
  scan_interval: 60
  method: GET
  sensor:
    - name: "${ip}"
      unique_id: ${ip}
      value_template: >
        {% if value_json is defined and value_json.stat is defined %}
          {{ value_json.stat | join(',') }}
        {% else %}
          Unavailable
        {% endif %}
`;
}
/*
# - resource: "http://192.168.1.150/sendAJAX?json=%7B%22t%22:127,%22v%22:0%7D" # t=127 is status
#   scan_interval: 60
#   method: GET
#   sensor:
#     - name: "ESP8266 Status"
#       unique_id: esp8266_status
#       # Convert/Parse the returned JSON (if you're getting a JSON response as it appears)
#       value_template: >
#         {% if value_json is defined and value_json.stat is defined %}
#           {{ value_json.stat | join(',') }}
#         {% else %}
#           Unavailable
#         {% endif %}
*/