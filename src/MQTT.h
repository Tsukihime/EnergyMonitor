#ifndef __MQTT_H
#define __MQTT_H

#include <stdint.h>

class MQTT {
public:
    static void init(const char *domain, uint16_t port, const char* login, const char* password);
    static void loop();
    static void publish(const char *topic, const char *value, bool retain = false);
    static void publish(const char* topic, const uint8_t* payload, unsigned int plength, bool retain = false);
    static void disconnect();
    static void sendMQTTDiscoveryConfig();

private:
    static void onConnect();
    static void messageArrived(char *p_topic, uint8_t *p_payload, unsigned int p_length);
};

#endif
