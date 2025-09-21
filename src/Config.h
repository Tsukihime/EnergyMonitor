#ifndef CONFIG_H
#define CONFIG_H

namespace Config {
    static uint32_t getChipId() {
        uint32_t chipId = 0;    
        for (int i = 0; i < 17; i += 8) {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
    
        return chipId;
    }

    static String getDeviceName() {
        return "EnergyMonitor";
    }

    static String getDeviceId() {
        return String(getChipId(), HEX);
    }

    static String getMqttPrefix() {
        return "home/EnergyMonitor_" + getDeviceId() + "/";
    }
};

#endif //CONFIG_H
