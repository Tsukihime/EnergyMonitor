#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#include <HardwareSerial.h>

#include "MQTT.h"
#include "Config.h"
#include "PowerMeter.h"
#include <ApSettingsManager.h>

Ticker ticker;
ApSettingsManager apManager;
HardwareSerial IndicatorSerial(1);

// (a b c d e f g .dp)
const uint8_t seg_patterns[10] = {
  0b0111111,   // 0   0b00111111 - a=bit0, g=bit6
  0b0000110,   // 1
  0b1011011,   // 2
  0b1001111,   // 3
  0b1100110,   // 4
  0b1101101,   // 5
  0b1111101,   // 6
  0b0000111,   // 7
  0b1111111,   // 8
  0b1101111    // 9
};

const uint8_t dp_mask = 0b10000000;

void clearDisplay() {
    uint8_t display[9] = {0};
    IndicatorSerial.write(display, 9);
}

void formatTo3Digits(float value, uint8_t* out, bool show_unit = true) {
    String str = String(value, 2);

    int idx = 0;
    for (int i = 0; i < str.length() && idx < 3; i++) {
        char c = str[i];

        if (c >= '0' && c <= '9') {
            out[idx++] = seg_patterns[c - '0'];
        } else if (c == '.') {
            out[idx - 1] |= dp_mask;
        }
    }

    if(show_unit) {
        out[2] |= dp_mask;
    }
}

void updateState() {
    static int calls = 0;

    if (PowerMeter::measure(
        apManager.getParameter("voltageCoeff").toFloat(),
        apManager.getParameter("currentCoeff").toFloat()
    )) {
        if (calls++ >= 5) {
            calls = 0;
            String state_topic = Config::getMqttPrefix() + "state";
            String payload = "{\"v_rms\":" + String(PowerMeter::getVrms(), 1) +
                            ",\"i_rms\":" + String(PowerMeter::getIrms(), 2) +
                            ",\"power\":" + String(PowerMeter::getPower(), 0) +
                            ",\"cos_phi\":" + String(PowerMeter::getCosPhi(), 3) +
                            ",\"frequency\":" + String(PowerMeter::getFrequency(), 1) + "}";

            MQTT::publish(state_topic.c_str(), payload.c_str());
        }

        String json = "{\"Voltage\":" + String(PowerMeter::getVrms(), 1) +
                ",\"Current\":" + String(PowerMeter::getIrms(), 2) +
                ",\"Power\":" + String(PowerMeter::getPower(), 0) +
                ",\"cos_phi\":" + String(PowerMeter::getCosPhi(), 3) +
                ",\"frequency\":" + String(PowerMeter::getFrequency(), 1) + "}";

        apManager.setLogJson(json);

        uint8_t display[9] = {0};
        formatTo3Digits(PowerMeter::getVrms(), &display[0]);
        formatTo3Digits(PowerMeter::getIrms(), &display[3]);
        formatTo3Digits(PowerMeter::getPower() / 1000.0, &display[6], false);

        IndicatorSerial.write(display, 9);
    } else {
        Serial.println("Измерение не выполнено!");
    }
}

void setup() {
    Serial.begin(115200);
    IndicatorSerial.begin(9600, SERIAL_8N1, -1, 21);
    clearDisplay();

    String customParameters = R"([{
        "title": "Настройки MQTT",
        "rows": [
            {"name": "mqttServer", "type": "text", "title": "Сервер:", "value": "192.168.0.1"},
            {"name": "mqttPort", "type": "number", "title": "Порт:", "value": "1883"},
            {"name": "mqttLogin", "type": "text", "title": "Логин:", "value": ""},
            {"name": "mqttPassword", "type": "password", "title": "Пароль:", "value": ""}
        ]
    },{ "title": "Коррекция измерений",
        "rows": [
            {"name": "currentCoeff", "type": "number", "title": "Ток:", "value": "1.0000"},
            {"name": "voltageCoeff", "type": "number", "title": "Напряжение:", "value": "1.0000"}
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

    ArduinoOTA.setHostname(Config::getDeviceName().c_str());
    ArduinoOTA.begin();
    ticker.attach(1, updateState);
    updateState();
}

void loop() {
    apManager.handle();
    ArduinoOTA.handle();
    MQTT::loop();
}
