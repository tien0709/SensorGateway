#ifndef INC_CLI_MANAGER_HPP_
#define INC_CLI_MANAGER_HPP_

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f4xx_hal_uart.h"
#ifdef __cplusplus
}
#endif

#include<string>
#include<vector>
#include<memory>
#include<map>
#include "IObserver.hpp"
#include "sensor_manager.hpp"

class ICLICommand {
public:
    virtual ~ICLICommand() = default;
    virtual std::string execute(const std::vector<std::string>& parameters) = 0;
    virtual std::string getHelp() const = 0;
};

class CLIManager : public IObserver<SensorData> {
private:
    std::map<std::string, std::unique_ptr<ICLICommand>> commands;
    osMessageQId cliCommandQueue;
    osSemaphoreId cliMutex;
    osThreadId cliTaskId;
    UART_HandleTypeDef* huart;
    SensorManager* sensorManager;
    SystemStatus systemStatus;

    std::string rxBuffer;
    std::string inputBuffer;
    static const size_t MAX_INPUT_LENGTH = 256;

    static void cliTask(void* parameter);
    void processCommand(const std::string& commandLine);
    std::vector<std::string> parseCommand(const std::string& commandLine);
    void sendResponse(const std::string& response);
    void updateSystemStatus();

public:
    CLIManager(UART_HandleTypeDef* uart, SensorManager* sensorMgr);
    ~CLIManager();

    void init();
    void registerCommand(const std::string& name, std::unique_ptr<ICLICommand> command);
    void handleUARTData(uint8_t* data, uint16_t size);
    void update(const SensorData& data) override;

    const SystemStatus& getSystemStatus() const { return systemStatus; }
};

// CLI Commands
class HelpCommand : public ICLICommand {
private:
    CLIManager* cliManager;

public:
    HelpCommand(CLIManager* manager) : cliManager(manager) {}

    std::string execute(const std::vector<std::string>& parameters) override {
        return "Available commands: help, status, reset, sensors, log, version\r\n";
    }

    std::string getHelp() const override {
        return "help - Show available commands\r\n";
    }
};

class StatusCommand : public ICLICommand {
private:
    CLIManager* cliManager;

public:
    StatusCommand(CLIManager* manager) : cliManager(manager) {}

    std::string execute(const std::vector<std::string>& parameters) override {
        const SystemStatus& status = cliManager->getSystemStatus();
        char buffer[512];
        snprintf(buffer, sizeof(buffer),
                "System Status:\r\n"
                "  State: %s\r\n"
                "  Uptime: %lu ms\r\n"
                "  Free Heap: %lu bytes\r\n"
                "  Active Sensors: %lu/%lu\r\n"
                "  Error Count: %lu\r\n"
                "  CPU Usage: %d%%\r\n",
                status.state == SystemState::RUNNING ? "RUNNING" : "IDLE",
                status.upTime,
                status.freeHeap,
                status.activeSensors,
                status.totalSensors,
                status.errorCount,
                status.cpuUsage);
        return std::string(buffer);
    }

    std::string getHelp() const override {
        return "status - Show system status information\r\n";
    }
};

class ResetCommand : public ICLICommand {
public:
    std::string execute(const std::vector<std::string>& parameters) override {
        SystemLogger::getInstance()->log(LogLevel::info, "System reset requested via CLI", "CLI");
        HAL_Delay(100); // Allow log to be sent
        HAL_NVIC_SystemReset();
        return "Resetting system...\r\n";
    }

    std::string getHelp() const override {
        return "reset - Reset the system\r\n";
    }
};

class SensorsCommand : public ICLICommand {
private:
    SensorManager* sensorManager;

public:
    SensorsCommand(SensorManager* manager) : sensorManager(manager) {}

    std::string execute(const std::vector<std::string>& parameters) override {
        if (parameters.empty()) {
            // Show all sensor data
            std::vector<SensorData> allData = sensorManager->getAllSensorData();
            std::string result = "Sensor Data:\r\n";

            for (const auto& data : allData) {
                char buffer[128];
                snprintf(buffer, sizeof(buffer),
                        "  Sensor %d (%s): %d [%lu]\r\n",
                        data.sensorId,
                        data.type == SensorType::TEMPERATURE ? "TEMP" : "UNKNOWN",
                        data.value,
                        data.timestamp);
                result += buffer;
            }
            return result;
        } else if (parameters[0] == "test") {
            bool testResult = sensorManager->performSelfTest();
            return testResult ? "Sensor self-test: PASSED\r\n" : "Sensor self-test: FAILED\r\n";
        } else if (parameters[0] == "reset") {
            sensorManager->resetAllSensors();
            return "All sensors reset\r\n";
        }

        return "Usage: sensors [test|reset]\r\n";
    }

    std::string getHelp() const override {
        return "sensors [test|reset] - Show sensor data or perform operations\r\n";
    }
};


#endif /* INC_CLI_MANAGER_HPP_ */
