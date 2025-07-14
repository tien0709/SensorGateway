#ifndef INC_CONFIG_MANAGER_HPP_
#define INC_CONFIG_MANAGER_HPP_

#ifdef __cplusplus
extern "C" {
#endif
#include<stdint.h>
#include "cmsis_os.h"
#ifdef __cplusplus
}
#endif

#include<string>

struct SystemConfig {
    uint32_t sensorReadInterval;
    uint32_t logLevel;
    uint32_t watchdogTimeout;
    uint32_t maxSensors;
    bool autoStart;
    std::string deviceName;

    SystemConfig() : sensorReadInterval(1000), logLevel(2), watchdogTimeout(5000),
                    maxSensors(10), autoStart(true), deviceName("SensorGateway") {}
};

class ConfigManager {
private:
    SystemConfig config;
    osSemaphoreId configMutex;

    static const uint32_t CONFIG_FLASH_ADDRESS = 0x08060000; // Last sector

    bool saveToFlash();
    bool loadFromFlash();
    uint32_t calculateChecksum(const SystemConfig& cfg);

public:
    ConfigManager();
    ~ConfigManager();

    void init();
    const SystemConfig& getConfig() const;
    void setConfig(const SystemConfig& newConfig);
    void resetToDefault();
    bool saveConfig();
    bool loadConfig();

    // Individual parameter setters
    void setSensorReadInterval(uint32_t interval);
    void setLogLevel(uint32_t level);
    void setWatchdogTimeout(uint32_t timeout);
    void setMaxSensors(uint32_t maxSensors);
    void setAutoStart(bool autoStart);
    void setDeviceName(const std::string& name);
};



#endif /* INC_CONFIG_MANAGER_HPP_ */
