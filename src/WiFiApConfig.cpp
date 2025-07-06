#include "WiFiApConfig.h"

DNSServer dnsServer;

unsigned long WiFiApConfig::apStartTime = 0;
String WiFiApConfig::wifiSsid;
String WiFiApConfig::wifiPassword;
unsigned long WiFiApConfig::apTimeout;
static unsigned long lastWiFiCheck = 0;
static const unsigned long wifiCheckInterval = 30000;

void WiFiApConfig::begin(const String& ssid, const String& password, const String& ap_ssid, const String& ap_password, unsigned long ap_timeout) {
    wifiSsid = ssid;
    wifiPassword = password;
    apTimeout = ap_timeout;

    WiFi.hostname(ap_ssid);

    if (!connectToWiFi()) {
        startAccessPoint(ap_ssid, ap_password);
    }
}

void WiFiApConfig::handle() {
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
        // Handling DNS queries for Captive Portal
        dnsServer.processNextRequest();

        // Checking the access point timer
        if (apStartTime > 0 && WiFi.softAPgetStationNum() == 0 && millis() - apStartTime > apTimeout) {
            Serial.println("Nobody connected to the access point for 3 minutes, rebooting...");
            ESP.restart();
        }
    } else if (WiFi.getMode() == WIFI_STA && millis() - lastWiFiCheck > wifiCheckInterval) {
        lastWiFiCheck = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wi-Fi connection lost, trying to reconnect...");
            connectToWiFi();
        }
    }
}

bool WiFiApConfig::connectToWiFi() {
    Serial.println("Connecting to Wi-Fi: " + wifiSsid);
    WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect to Wi-Fi");
        return false;
    }

    Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
    lastWiFiCheck = millis();
    return true;
}

void WiFiApConfig::startAccessPoint(const String& apSSID, const String& apPassword) {
    Serial.println("Starting access point");
    WiFi.disconnect();
    WiFi.softAP(apSSID.c_str(), apPassword.c_str());
    Serial.println("Access point: " + apSSID + ", IP: " + WiFi.softAPIP().toString());

    // Configure DNS server for Captive Portal
    dnsServer.start(53, "*", WiFi.softAPIP());

    // Remember access point start time
    apStartTime = millis();
}
