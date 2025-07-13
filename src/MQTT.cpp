#include "mqtt.h"

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#else
    #error "Unsupported platform! Please use ESP8266 or ESP32."
#endif

#include <PubSubClient.h>
#include "Config.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient);

String mqttDomain;
String mqttLogin;
String mqttPassword;

void MQTT::init(const char *domain, uint16_t port, const char *login, const char *password) {
    mqttDomain = domain;
    mqttLogin = login;
    mqttPassword = password;
    client.setServer(mqttDomain.c_str(), port);
    client.setCallback(messageArrived);
}

void MQTT::publish(const char *topic, const char *value, bool retain) {
    Serial.println("MQTT: Publish: " + String(topic) + " => " + value);
    client.publish(topic, value, retain);
}

void MQTT::publish(const char* topic, const uint8_t* payload, unsigned int plength, bool retain) {
    Serial.println("MQTT: Publish binary data: " + String(topic));
    client.publish(topic, payload, plength, retain);
}

void MQTT::disconnect() {
    Serial.println("MQTT: Closing connection");
    client.disconnect();
}

String getMQTTErrorDescription(int state) {
    switch (state) {
        case -4: return "Connection timeout";
        case -3: return "Connection lost";
        case -2: return "Connect failed";
        case -1: return "Disconnected";
        case 0:  return "Connected";
        case 1:  return "Bad protocol";
        case 2:  return "Bad client ID";
        case 3:  return "Server unavailable";
        case 4:  return "Bad credentials";
        case 5:  return "Unauthorized";
        default: return "Unknown error (" + String(state) + ")";
    }
}

void MQTT::loop() {
    static unsigned long lastReconnectAttempt = 0;

    if (!client.connected() && (WiFi.status() == WL_CONNECTED)) {
        if (millis() - lastReconnectAttempt >= 5000) {
            Serial.println("MQTT: Attempting connection...");
            String mqttClientId = Config::getDeviceName() + "-" + Config::getDeviceId();
            if (client.connect(mqttClientId.c_str(), mqttLogin.c_str(), mqttPassword.c_str())) {
                Serial.println("MQTT: Connected!");
                onConnect();
            } else {
                Serial.println("MQTT: Connection failed: " + getMQTTErrorDescription(client.state()) + ", will try again later");
            }
            lastReconnectAttempt = millis();
        }
    }
    client.loop();
}

void MQTT::messageArrived(char *p_topic, uint8_t *p_payload, unsigned int p_length) {
    char c_payload[p_length + 1];
    memcpy(c_payload, p_payload, p_length);
    c_payload[p_length] = 0;

    Serial.println("MQTT: Received: " + String(p_topic) + " => " + c_payload);
}

void MQTT::onConnect() {
    // client.subscribe("home/BMP280_indoor");
    sendMQTTDiscoveryConfig();
}

void MQTT::sendMQTTDiscoveryConfig() {
    String state_topic = Config::getMqttPrefix() + "state";
    String device_object = R"({
        "identifiers": [")" + Config::getDeviceId() + R"("],
        "name": "Energy Monitor",
        "manufacturer": "Tsukihime",
        "model": "ESP32-PowerMeter",
        "sw_version": "0.1"
    })";

    // Voltage sensor
    String voltage_discovery_topic = "homeassistant/sensor/powermeter_voltage/config";
    String voltage_discovery_message = R"({
        "device": )" + device_object + R"(,
        "unique_id": "powermeter_voltage_)" + Config::getDeviceId() + R"(",
        "name": "Voltage",
        "state_topic": ")" + state_topic + R"(",
        "unit_of_measurement": "V",
        "device_class": "voltage",
        "value_template": "{{ value_json.v_rms }}"
    })";
    client.publish(voltage_discovery_topic.c_str(), voltage_discovery_message.c_str(), true);

    // Current sensor
    String current_discovery_topic = "homeassistant/sensor/powermeter_current/config";
    String current_discovery_message = R"({
        "device": )" + device_object + R"(,
        "unique_id": "powermeter_current_)" + Config::getDeviceId() + R"(",
        "name": "Current",
        "state_topic": ")" + state_topic + R"(",
        "unit_of_measurement": "A",
        "device_class": "current",
        "value_template": "{{ value_json.i_rms }}"
    })";
    client.publish(current_discovery_topic.c_str(), current_discovery_message.c_str(), true);

    // Power sensor
    String power_discovery_topic = "homeassistant/sensor/powermeter_power/config";
    String power_discovery_message = R"({
        "device": )" + device_object + R"(,
        "unique_id": "powermeter_power_)" + Config::getDeviceId() + R"(",
        "name": "Power",
        "state_topic": ")" + state_topic + R"(",
        "unit_of_measurement": "W",
        "device_class": "power",
        "value_template": "{{ value_json.power }}"
    })";
    client.publish(power_discovery_topic.c_str(), power_discovery_message.c_str(), true);

    // Power factor (cos phi) sensor
    String cos_phi_discovery_topic = "homeassistant/sensor/powermeter_cos_phi/config";
    String cos_phi_discovery_message = R"({
        "device": )" + device_object + R"(,
        "unique_id": "powermeter_cos_phi_)" + Config::getDeviceId() + R"(",
        "name": "Power Factor",
        "state_topic": ")" + state_topic + R"(",
        "unit_of_measurement": "",
        "device_class": "power_factor",
        "value_template": "{{ value_json.cos_phi }}"
    })";
    client.publish(cos_phi_discovery_topic.c_str(), cos_phi_discovery_message.c_str(), true);
}
