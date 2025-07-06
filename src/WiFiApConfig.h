#ifndef WIFI_AP_CONFIG_H
#define WIFI_AP_CONFIG_H

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
#elif defined(ESP32)
    #include <WiFi.h>
#else
    #error "Unsupported platform! Please use ESP8266 or ESP32."
#endif

#include <DNSServer.h>

class WiFiApConfig {
public:
    static void begin(
        const String& ssid,
        const String& password,
        const String& ap_ssid = "ESP WiFI AP",
        const String& ap_password  = "",
        unsigned long ap_timeout = 180000); // 3 минуты

    static void handle();

private:
    static bool connectToWiFi();
    static void startAccessPoint(const String& apSSID, const String& apPassword);

    static String wifiSsid;
    static String wifiPassword;

    static unsigned long apStartTime;
    static unsigned long apTimeout;
};

#endif
