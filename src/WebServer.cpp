#include "WebServer.h"
#include "Settings.h"
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"

AsyncWebServer WebServer::server(80);
String parameters = "{}";

void WebServer::begin() {
    if (!LittleFS.begin()) {
        Serial.println("WebServer: LittleFS Mount Error");
        return;
    }

    server.on("/", HTTP_GET, handleRoot);
    server.onNotFound(handleNotFound);
    server.on("/scan", HTTP_GET, handleScan);
    server.on("/settings", HTTP_GET, handleSettings);
    server.on("/parameters", HTTP_GET, handleParameters);
    server.on("/save", HTTP_POST, handleSave);
    server.serveStatic("/", LittleFS, "/");
    server.begin();
}

void WebServer::setParameters(String& params) {
    parameters = params;
}

void WebServer::handleRoot(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void WebServer::handleNotFound(AsyncWebServerRequest *request) {
    Serial.println("Intercepting request: " + request->url() + ", redirecting to the main page");
    request->redirect("http://" + WiFi.softAPIP().toString() + "/");
}

void WebServer::handleScan(AsyncWebServerRequest *request) {
    int n = WiFi.scanComplete();

    if (n >= 0) {
        Serial.printf("Scan complete, found %d networks\n", n);
        String json = "[";
        for (int i = 0; i < n; ++i) {
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + WiFi.RSSI(i) + "}";
            if (i < n - 1) json += ",";
        }
        json += "]";

        request->send(200, "application/json", json);
        WiFi.scanDelete();
        return;
    }

    if (n == -1) {
        Serial.println("Scan in progress, returning wait state");
    } else {
        Serial.println("No scan in progress, starting new scan");
        WiFi.scanNetworks(true, true);
    }

    String json = R"({"state": "wait"})";
    request->send(200, "application/json", json);
}

void WebServer::handleParameters(AsyncWebServerRequest *request) {
    request->send(200, "application/json", parameters);
}

void WebServer::handleSettings(AsyncWebServerRequest *request) {
String json = R"({
"header": ")" + Config::getDeviceName() + R"(",
"wifiSsid": ")" + String(Settings::getWifiSsid()) + R"(",
"wifiPassword": ")" + String(Settings::getWifiPassword()) + R"(",
"sections": [{
  "title": "Настройки MQTT",
  "rows": [
    {"name": "mqttServer", "type": "text", "title": "Сервер:",
    "value": ")" + String(Settings::getMqttServer()) + R"("},
    {"name": "mqttPort", "type": "number", "title": "Порт:",
    "value": ")" + String(Settings::getMqttPort()) + R"("},
    {"name": "mqttLogin", "type": "text", "title": "Логин:",
    "value": ")" + String(Settings::getMqttUsername()) + R"("},
    {"name": "mqttPassword", "type": "password", "title": "Пароль:",
    "value": ")" + String(Settings::getMqttPassword()) + R"("}
  ]
}]
})";
    request->send(200, "application/json", json);
}

void WebServer::handleSave(AsyncWebServerRequest *request) {
    if (request->hasParam("wifiSsid", true)) Settings::setWifiSsid(request->getParam("wifiSsid", true)->value().c_str());
    if (request->hasParam("wifiPassword", true)) Settings::setWifiPassword(request->getParam("wifiPassword", true)->value().c_str());
    if (request->hasParam("mqttServer", true)) Settings::setMqttServer(request->getParam("mqttServer", true)->value().c_str());
    if (request->hasParam("mqttPort", true)) Settings::setMqttPort(request->getParam("mqttPort", true)->value().toInt());
    if (request->hasParam("mqttLogin", true)) Settings::setMqttUsername(request->getParam("mqttLogin", true)->value().c_str());
    if (request->hasParam("mqttPassword", true)) Settings::setMqttPassword(request->getParam("mqttPassword", true)->value().c_str());
    Settings::save();

    request->send(200, "text/plain", "Settings saved! Rebooting...");

    request->onDisconnect([]() {
        Serial.println("Settings saved! Rebooting...");
        ESP.restart();
    });
}
