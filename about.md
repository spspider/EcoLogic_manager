# EcoLogic Manager: Comprehensive Overview

## What is EcoLogic Manager?
EcoLogic Manager is a highly flexible, open-source automation platform designed for the ESP8266 microcontroller. It enables users to create smart, connected devices that can be controlled, monitored, and automated via a web-based HTTP interface, MQTT, and integration with popular home automation systems like Home Assistant and Node-RED. The system is tailored for DIY enthusiasts, makers, and integrators who want to manage electrical loads, sensors, and actuators in a home or industrial environment without deep coding knowledge.

---

## Key Features
- **Web-Based Control:** Intuitive HTTP web interface for real-time device management, configuration, and monitoring from any browser.
- **Pin Configuration:** Flexible pin setup for digital/analog I/O, PWM, sensors (DHT, DS18B20, MQ7, etc.), relays, and more.
- **MQTT Support:** Full MQTT client for integration with cloud brokers and local servers, supporting both status reporting and remote control.
- **Home Automation Integration:** Ready for Home Assistant, Node-RED, and other automation platforms via MQTT and HTTP REST APIs.
- **IR & RF Support:** Receive and transmit IR (infrared) and 433MHz RF codes for remote control and automation.
- **Email & Notification:** Send email alerts directly from the device for alarms or status changes.
- **Modular & Extensible:** Easily add new sensors, actuators, and logic via configuration files and modular code structure.
- **Data Storage:** Uses local files (SPIFFS/LittleFS) for persistent configuration and state.

---

## Typical Use Cases
| Scenario                        | Description                                                                                 |
|----------------------------------|---------------------------------------------------------------------------------------------|
| Smart Lighting                   | Switch, dim, or automate lights via web, MQTT, or physical buttons.                         |
| Environmental Monitoring         | Read temperature, humidity, air quality, and report to dashboards or trigger automations.   |
| Appliance Control                | Remotely turn on/off appliances, schedule operations, or automate based on sensor input.     |
| Security & Access                | Integrate door sensors, motion detectors, and control locks or alarms.                      |
| IR/RF Remote Replication         | Learn and replay IR/RF codes to control TVs, fans, or other devices.                        |
| Integration with Home Assistant  | Expose all sensors and controls as entities for advanced automation and visualization.       |
| Node-RED Flows                   | Use MQTT/HTTP endpoints in Node-RED for custom logic, dashboards, and notifications.         |

---

## Device Control & Integration
### HTTP API
- **Direct Pin Control:**
  - Example: `http://<device_ip>/sendAJAX?data={"t":0,"v":1}` — Set pin 0 to HIGH.
  - Read status: `http://<device_ip>/aRest?data={stat:0}`
- **IR/RF Code Management:**
  - Learn, send, and manage codes via HTTP endpoints.
- **Email Sending:**
  - Example: `http://<device_ip>/sendEmail?Email=MessageText`
- **Reboot/Time Sync:**
  - Example: `http://<device_ip>/function?data={reboot:1}`

### MQTT Topics
- **Status Topic:** `<deviceID>/<pin>/status` — Publishes pin state changes.
- **Control Topic:** `<deviceID>/<pin>` — Receives commands to change pin state.
- **Customizable Prefix:** All topics can be prefixed (e.g., `/IoTmanager/dev01/0/status`).

### Home Assistant & Node-RED
- **Discovery:** Manual or automatic entity creation using MQTT topics.
- **Automation:** Use device topics in automations, flows, and dashboards.

---

## Pin Setup & Configuration
The system allows detailed configuration of each pin, including mode, function, widget type, and default state. This is managed via the web interface (`pin_setup.htm`) and stored in `pin_setup.txt`.

### Supported Pin Modes
| Mode Index | Mode Name         | Description                                                      |
|------------|------------------|------------------------------------------------------------------|
| 0          | no               | Disabled                                                         |
| 1          | in               | Digital input (with inversion option)                            |
| 2          | out              | Digital output (relay, LED, etc.)                                |
| 3          | PWM              | PWM output for dimming, speed control                            |
| 4          | ADC              | Analog input (A0)                                                |
| 5          | low. PWM         | Low-frequency PWM                                                |
| 6          | DHT 1.1 Temp     | DHT11 temperature sensor                                         |
| 7          | power MQ7        | MQ7 gas sensor power                                             |
| 8          | DHT 1.1 Mist     | DHT11 humidity sensor                                            |
| 9          | remote http      | Remote HTTP control                                              |
| 10         | power meter      | Power metering                                                   |
| 11         | as5600           | Magnetic encoder                                                 |
| 12         | MAC address      | MAC address reading                                              |
| 13         | EncA             | Encoder channel A                                                |
| 14         | EncB             | Encoder channel B                                                |
| 15         | ads1115          | ADS1115 ADC module                                               |
| 16         | ds18b20          | DS18B20 temperature sensor                                       |

### Pin Setup Table Example
| Pin Mode | Pin | Description      | Widget   | IR Btn | Default | Notes                       |
|----------|-----|------------------|----------|--------|---------|-----------------------------|
| out      | 5   | Relay1           | switch   | 255    | 0       | Controls main relay         |
| in       | 4   | Door Sensor      | button   | 255    | 1       | Normally closed contact     |
| PWM      | 12  | LED Strip        | slider   | 255    | 0       | Dimmable LED               |
| DHT 1.1  | 14  | Temp Sensor      | chart    | 255    | 0       | Reports temperature         |

- **Widget Types:** switch, button, progress, chart, data, etc.
- **IR Btn:** Link to IR remote button (if used).
- **Default:** Initial state after boot.

---

## File-Based Configuration
All settings are stored in local files, making backup, migration, and manual editing easy. Key files include:
- `pin_setup.txt` — Pin configuration
- `other_setup.txt` — MQTT, email, and device settings
- `wifilist.txt` — WiFi credentials
- `ConditionX.txt` — Automation conditions
- `stat.txt` — Current pin states

---

## How to Connect & Use
1. **Power the Device:** Connect your ESP8266 to power and network.
2. **Access Web UI:** Open a browser and navigate to the device's IP address.
3. **Configure Pins:** Use the Pin Setup page to assign functions to each pin.
4. **Integrate with Automation:** Set up MQTT in `other_setup.txt` and connect to Home Assistant or Node-RED.
5. **Control & Monitor:** Use the web UI, MQTT, or HTTP API to control and monitor your device.
6. **Advanced:** Use IR/RF, email, and custom automations as needed.

---

## Integration Scenarios
| Platform         | Method         | Example/Notes                                              |
|------------------|---------------|------------------------------------------------------------|
| Home Assistant   | MQTT          | Add MQTT sensor/switch with device topics                  |
| Node-RED         | MQTT/HTTP     | Use device as node in flows, trigger automations           |
| Custom Web App   | HTTP API      | Send AJAX/REST requests to device endpoints                |
| IR/RF Remotes    | HTTP/IR/RF    | Learn and replay codes for remote control                  |
| Email Alerts     | HTTP/SMTP     | Device sends email on alarm or event                       |

---

## Extensibility & Customization
- **Add New Sensors/Actuators:** Edit pin setup and upload new code modules.
- **Custom Logic:** Use `ConditionX.txt` files for automation rules.
- **Backup/Restore:** Copy configuration files for easy migration.

---

## Security & Best Practices
- Change default passwords and WiFi credentials.
- Use secure MQTT brokers (TLS supported).
- Regularly backup configuration files.

---

## Conclusion
EcoLogic Manager transforms the ESP8266 into a powerful, web-managed automation controller. Its modular, file-driven approach and broad integration support make it ideal for smart home, industrial, and educational projects. Whether you need simple remote switching or complex automation, EcoLogic Manager provides a robust, extensible foundation.
