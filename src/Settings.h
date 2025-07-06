#ifndef SETTINGS_H
#define SETTINGS_H

#include "stdint.h"

class Settings {
private:
    static struct Config {
        uint16_t magic;
        char wifiSsid[33];        // 32 chars + null terminator
        char wifiPassword[65];    // 64 chars + null terminator
        char mqttServer[255];
        char mqttUsername[255];
        char mqttPassword[255];
        uint16_t mqttPort;
    } config;

public:
    static void load();
    static void save();

    static const char* getMqttServer();
    static void setMqttServer(const char* server);

    static uint16_t getMqttPort();
    static void setMqttPort(uint16_t port);

    static const char* getMqttUsername();
    static void setMqttUsername(const char* username);

    static const char* getMqttPassword();
    static void setMqttPassword(const char* password);

    static const char* getWifiSsid();
    static void setWifiSsid(const char* ssid);

    static const char* getWifiPassword();
    static void setWifiPassword(const char* password);
};

#endif // SETTINGS_H
