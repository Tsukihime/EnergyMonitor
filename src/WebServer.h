#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>

class WebServer {
public:
    static void begin();
    static void setParameters(String& params);

private:
    static AsyncWebServer server;
    static void handleRoot(AsyncWebServerRequest *request);
    static void handleNotFound(AsyncWebServerRequest *request);
    static void handleScan(AsyncWebServerRequest *request);
    static void handleParameters(AsyncWebServerRequest *request);
    static void handleSettings(AsyncWebServerRequest *request);
    static void handleSave(AsyncWebServerRequest *request);
};

#endif
