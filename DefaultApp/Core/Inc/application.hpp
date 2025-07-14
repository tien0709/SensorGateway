#ifndef INC_APPLICATION_HPP_
#define INC_APPLICATION_HPP_

#include <memory>
#include "system_logger.hpp"
#include "sensor_manager.hpp"
#include "cli_manager.hpp"
#include "system_monitor.hpp"
#include "config_manager.hpp"
#include "data_buffer.hpp"

class Application {
private:
    // Core components
    std::unique_ptr<SystemLogger> logger;
    std::unique_ptr<SensorManager> sensorManager;
    std::unique_ptr<CLIManager> cliManager;
    std::unique_ptr<SystemMonitor> systemMonitor;
    std::unique_ptr<ConfigManager> configManager;
    std::unique_ptr<DataStorage> dataStorage;

    // Hardware handles
    SPI_HandleTypeDef* hspi;
    UART_HandleTypeDef* huartCLI;
    UART_HandleTypeDef* huartLog;

    // Application state
    bool isInitialized;
    bool isRunning;

    void initializeHardware();
    void initializeComponents();
    void startComponents();
    void registerSensors();

public:
    Application(SPI_HandleTypeDef* spi, UART_HandleTypeDef* uartCLI, UART_HandleTypeDef* uartLog);
    ~Application();

    void init();
    void run();
    void stop();

    // Interrupt handlers
    void handleUARTInterrupt(UART_HandleTypeDef* huart, uint8_t* data, uint16_t size);
    void handleSPIInterrupt(SPI_HandleTypeDef* hspi);

    // Getters for components
    SystemLogger* getLogger() const { return logger.get(); }
    SensorManager* getSensorManager() const { return sensorManager.get(); }
    CLIManager* getCLIManager() const { return cliManager.get(); }
    SystemMonitor* getSystemMonitor() const { return systemMonitor.get(); }
    ConfigManager* getConfigManager() const { return configManager.get(); }
    DataStorage* getDataStorage() const { return dataStorage.get(); }
};


#endif /* INC_APPLICATION_HPP_ */
