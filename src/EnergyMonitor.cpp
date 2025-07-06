#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Ticker.h>

#include "Settings.h"
#include "WebServer.h"
#include "WiFiApConfig.h"
#include "MQTT.h"
#include "Config.h"
#include "PowerMeter.h"

Ticker ticker;

void updateState() {
    // Измеряем мощность
    if (PowerMeter::measure()) {
        String state_topic = Config::getMqttPrefix() + "state";
        String payload = "{\"v_rms\":" + String(PowerMeter::getVrms(), 1) +
                        ",\"i_rms\":" + String(PowerMeter::getIrms(), 2) +
                        ",\"power\":" + String(PowerMeter::getP(), 0) +
                        ",\"cos_phi\":" + String(PowerMeter::getCosPhi(), 3) + "}";
        MQTT::publish(state_topic.c_str(), payload.c_str());

        payload = "{\"Voltage\":" + String(PowerMeter::getVrms(), 1) +
                ",\"Current\":" + String(PowerMeter::getIrms(), 2) +
                ",\"Power\":" + String(PowerMeter::getP(), 0) +
                ",\"Cos φ\":" + String(PowerMeter::getCosPhi(), 3) + "}";

        WebServer::setParameters(payload);
    } else {
        // Ошибка измерения
        Serial.println("Измерение не выполнено!");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    Settings::load();

    WiFiApConfig::begin(
        Settings::getWifiSsid(),
        Settings::getWifiPassword(),
        Config::getDeviceName()
    );

    WebServer::begin();

    MQTT::init(
        Settings::getMqttServer(), 
        Settings::getMqttPort(), 
        Settings::getMqttUsername(), 
        Settings::getMqttPassword()
    );

    ArduinoOTA.begin();
    ticker.attach(5, updateState);
}

void loop() {
    WiFiApConfig::handle();
    ArduinoOTA.handle();
    MQTT::loop();
}
