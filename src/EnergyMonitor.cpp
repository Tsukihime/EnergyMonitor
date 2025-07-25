#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Ticker.h>

#include "MQTT.h"
#include "Config.h"
#include "PowerMeter.h"
#include <ApSettingsManager.h>

Ticker ticker;
ApSettingsManager apManager;

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

        apManager.setLogJson(payload);
    } else {
        // Ошибка измерения
        Serial.println("Измерение не выполнено!");
    }
}

void setup() {
    Serial.begin(115200);

    String customParameters = R"([{
        "title": "Настройки MQTT",
        "rows": [
            {"name": "mqttServer", "type": "text", "title": "Сервер:", "value": "192.168.0.1"},
            {"name": "mqttPort", "type": "number", "title": "Порт:", "value": "1883"},
            {"name": "mqttLogin", "type": "text", "title": "Логин:", "value": ""},
            {"name": "mqttPassword", "type": "password", "title": "Пароль:", "value": ""}
        ]
    }])";

    apManager.begin(Config::getDeviceName());
    apManager.setCustomParameters(customParameters);

    MQTT::init(
        apManager.getParameter("mqttServer").c_str(), 
        apManager.getParameter("mqttPort").toInt(), 
        apManager.getParameter("mqttLogin").c_str(), 
        apManager.getParameter("mqttPassword").c_str()
    );

    ArduinoOTA.begin();
    ticker.attach(5, updateState);
}

void loop() {
    apManager.handle();
    ArduinoOTA.handle();
    MQTT::loop();
}
