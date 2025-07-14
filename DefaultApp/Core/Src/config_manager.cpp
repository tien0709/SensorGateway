#include "config_manager.hpp"
#include "cmsis_os.h"
#include "system_logger.hpp"

ConfigManager::ConfigManager() {
    configMutex = xSemaphoreCreateMutex();
}

ConfigManager::~ConfigManager() {
    vSemaphoreDelete(configMutex);
}

void ConfigManager::init() {
    if (!loadFromFlash()) {
        // Use default configuration
        SystemLogger::getInstance()->log(LogLevel::warning, "Using default configuration", "CONFIG");
    }
    SystemLogger::getInstance()->log(LogLevel::info, "Configuration Manager initialized", "CONFIG");
}

const SystemConfig& ConfigManager::getConfig() const {
    return config;
}

void ConfigManager::setConfig(const SystemConfig& newConfig) {
    if (xSemaphoreTake(configMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        config = newConfig;
        osSemaphoreRelease(configMutex);
        SystemLogger::getInstance()->log(LogLevel::info, "Configuration updated", "CONFIG");
    }
}

void ConfigManager::resetToDefault() {
    if (xSemaphoreTake(configMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        config = SystemConfig();
        osSemaphoreRelease(configMutex);
        SystemLogger::getInstance()->log(LogLevel::info, "Configuration reset to default", "CONFIG");
    }
}

bool ConfigManager::saveToFlash() {
    // Implement flash save logic
    // This would involve erasing the flash sector and writing the config
    // For now, return true as placeholder
    return true;
}

bool ConfigManager::loadFromFlash() {
    // Implement flash load logic
    // This would involve reading from flash and validating checksum
    // For now, return false to use defaults
    return false;
}

