#include "Settings.h"
#include <EEPROM.h>
#include <cstring>

Settings::Config Settings::config;

const uint16_t MAGIC = 0xC0DE;

void Settings::load() {
    EEPROM.begin(sizeof(config));
    EEPROM.get(0, config);
    EEPROM.end();
    if (config.magic != MAGIC) {
        config.magic = MAGIC;
        config.mqttPort = 1883;
        strcpy(config.mqttServer, "127.0.0.1");
        strcpy(config.mqttUsername, "");
        strcpy(config.mqttPassword, "");
        strcpy(config.wifiSsid, "");
        strcpy(config.wifiPassword, "");
    }
}

void Settings::save() {
    EEPROM.begin(sizeof(config));
    EEPROM.put(0, config);
    EEPROM.commit();
    EEPROM.end();
}

const char* Settings::getMqttServer() {
    return config.mqttServer;
}

void Settings::setMqttServer(const char* server) {
    strncpy(config.mqttServer, server, sizeof(config.mqttServer));
    config.mqttServer[sizeof(config.mqttServer) - 1] = '\0';
}

uint16_t Settings::getMqttPort() {
    return config.mqttPort;
}

void Settings::setMqttPort(uint16_t port) {
    config.mqttPort = port;
}

const char* Settings::getMqttUsername() {
    return config.mqttUsername;
}

void Settings::setMqttUsername(const char* username) {
    strncpy(config.mqttUsername, username, sizeof(config.mqttUsername));
    config.mqttUsername[sizeof(config.mqttUsername) - 1] = '\0';
}

const char* Settings::getMqttPassword() {
    return config.mqttPassword;
}

void Settings::setMqttPassword(const char* password) {
    strncpy(config.mqttPassword, password, sizeof(config.mqttPassword));
    config.mqttPassword[sizeof(config.mqttPassword) - 1] = '\0';
}

const char* Settings::getWifiSsid() {
    return config.wifiSsid;
}

void Settings::setWifiSsid(const char* ssid) {
    strncpy(config.wifiSsid, ssid, sizeof(config.wifiSsid));
    config.wifiSsid[sizeof(config.wifiSsid) - 1] = '\0';
}

const char* Settings::getWifiPassword() {
    return config.wifiPassword;
}

void Settings::setWifiPassword(const char* password) {
    strncpy(config.wifiPassword, password, sizeof(config.wifiPassword));
    config.wifiPassword[sizeof(config.wifiPassword) - 1] = '\0';
}
