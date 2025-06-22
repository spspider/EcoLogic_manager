#if defined(USE_PICOMQTT)
#include <PicoMQTT.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <ArduinoJson.h>

// Use global variables for credentials, loaded from config
extern char mqttServerName[60];
extern unsigned int mqttport;
extern char mqttuser[15];
extern char mqttpass[15];
extern FS *fileSystem;
bool picoMqtt_config_loaded = false;

PicoMQTT::Client* mqtt = nullptr;

unsigned long last_publish_time = 0;
int greeting_number = 1;

bool load_picoMqtt_config() {
    File configFile = fileSystem->open("/other_setup.txt", "r");
    if (!configFile) {
        Serial.println("Failed to open /other_setup.txt for MQTT config");
        return false;
    }
    DynamicJsonDocument jsonDocument(1024);
    DeserializationError error = deserializeJson(jsonDocument, configFile);
    configFile.close();
    if (error) {
        Serial.println("Failed to parse JSON! load_picoMqtt_config");
        return false;
    }
    if (jsonDocument.containsKey("mqttServerName"))
        strncpy(mqttServerName, jsonDocument["mqttServerName"], sizeof(mqttServerName) - 1);
    if (jsonDocument.containsKey("mqttport"))
        mqttport = jsonDocument["mqttport"];
    if (jsonDocument.containsKey("mqttuser"))
        strncpy(mqttuser, jsonDocument["mqttuser"], sizeof(mqttuser) - 1);
    if (jsonDocument.containsKey("mqttpass"))
        strncpy(mqttpass, jsonDocument["mqttpass"], sizeof(mqttpass) - 1);
    picoMqtt_config_loaded = true;
    Serial.printf("MQTT config loaded: server=%s, port=%d, user=%s\n", mqttServerName, mqttport, mqttuser);
    return true;
}

bool setup_picoMqtt() {
    if (!picoMqtt_config_loaded) {
        load_picoMqtt_config();
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected, MQTT will not start");
        return false;
    }
    if (mqtt) delete mqtt;
    mqtt = new PicoMQTT::Client(mqttServerName, mqttport, mqttuser, mqttpass);
    // Subscribe to all topics for each widget
    for (uint8_t i = 0; i < nWidgets; i++) {
        char topic[32];
        snprintf(topic, sizeof(topic), "%s/%d", deviceID, i);
        mqtt->subscribe(topic, [](const char * topic, const char * payload) {
            const char *lastSlash = strrchr(topic, '/');
            char idx = 0;
            if (lastSlash && *(lastSlash + 1) != '\0') {
                int temp = atoi(lastSlash + 1);
                idx = (char)temp;
            }
            int newValue = atoi(payload);
            callback_socket(idx, newValue);
            Serial.printf("callback: %d Payload: %d\n", (int)idx, newValue);
        });
    }
    // Add connection state callbacks
    mqtt->connected_callback = [] {
        Serial.println("MQTT connected");
    };
    mqtt->disconnected_callback = [] {
        Serial.println("MQTT disconnected");
    };
    mqtt->begin();
    Serial.println("PicoMQTT client started");
    return true;
}

void loop_picoMqtt() {
    if (!mqtt) return;
    mqtt->loop();
    // Publish a greeting message every 3 seconds.
    if (millis() - last_publish_time >= 3000) {
        String topic = "picomqtt/esp-" + WiFi.macAddress();
        String message = "Hello #" + String(greeting_number++);
        Serial.printf("Publishing message in topic '%s': %s\n", topic.c_str(), message.c_str());
        mqtt->publish(topic, message);
        last_publish_time = millis();
    }
}
#endif
