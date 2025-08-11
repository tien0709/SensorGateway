#include "cli_manager.hpp"
#include "cmsis_os.h"
#include<sstream>
#include"common_variables.hpp"

CLIManager::CLIManager(UART_HandleTypeDef* uart, SensorManager* sensorMgr)
    : huart(uart), sensorManager(sensorMgr) {

	osMessageQDef(cliQueueDef, CLI_CMD_QUEUE_SIZE, uint32_t);
	cliCommandQueue = osMessageCreate(osMessageQ(cliQueueDef), NULL);

	osSemaphoreDef(cliMutexDef);

	cliMutex = osSemaphoreCreate(osSemaphore(cliMutexDef), 1);

    // Initialize system status
    systemStatus.state = SystemState::IDLE;
    systemStatus.upTime = 0;
    systemStatus.freeHeap = 0;
    systemStatus.totalSensors = 0;
    systemStatus.activeSensors = 0;
    systemStatus.errorCount = 0;
    systemStatus.cpuUsage = 0.0f;
}

CLIManager::~CLIManager() {
    osMessageDelete(cliCommandQueue);
    osMutexDelete(cliMutex);
}

void CLIManager::init() {
    // Register default commands
    registerCommand("help", std::make_unique<HelpCommand>(this));
    registerCommand("status", std::make_unique<StatusCommand>(this));
    registerCommand("reset", std::make_unique<ResetCommand>());
    registerCommand("sensors", std::make_unique<SensorsCommand>(sensorManager));

    // Create CLI task
    osThreadDef(cliTaskDef, cliTask, osPriorityNormal, 1, 512);
    cliTaskId = osThreadCreate(osThread(cliTaskDef), this);

    // Send welcome message
    sendResponse("STM32F411 Sensor Gateway v1.0\r\nType 'help' for available commands.\r\n> ");

    SystemLogger::getInstance()->log(LogLevel::info, "CLI Manager initialized", "CLI_MGR");
}

void CLIManager::registerCommand(const std::string& name, std::unique_ptr<ICLICommand> command) {
    if (xSemaphoreTake(cliMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        commands[name] = std::move(command);
        xSemaphoreGive(cliMutex);
    }
}

void CLIManager::cliTask(const void* parameter) {
    CLIManager* cliManager = static_cast<CLIManager*>(const_cast<void*>(parameter));
    CLICommand cmd;

    while (true) {
        if (xQueueReceive(cliManager->cliCommandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            cliManager->processCommand(cmd.command);
        }

        cliManager->updateSystemStatus();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void CLIManager::processCommand(const std::string& commandLine) {
    std::vector<std::string> tokens = parseCommand(commandLine);

    if (tokens.empty()) {
        sendResponse("> ");
        return;
    }

    std::string commandName = tokens[0];
    std::vector<std::string> parameters(tokens.begin() + 1, tokens.end());

    if (xSemaphoreTake(cliMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        auto it = commands.find(commandName);
        if (it != commands.end()) {
//            try {
                std::string response = it->second->execute(parameters);
                sendResponse(response);
//            } catch (const std::exception& e) {
//                sendResponse("Error executing command: " + std::string(e.what()) + "\r\n");
//                SystemLogger::getInstance()->logError("CLI command error: " + std::string(e.what()), "CLI");
//            }
        } else {
            sendResponse("Unknown command: " + commandName + "\r\n");
        }
        xSemaphoreGive(cliMutex);
    }

    sendResponse("> ");
}

std::vector<std::string> CLIManager::parseCommand(const std::string& commandLine) {
    std::vector<std::string> tokens;
    std::istringstream iss(commandLine);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

void CLIManager::sendResponse(const std::string& response) {
    if (huart != nullptr) {
        HAL_UART_Transmit(huart, (uint8_t*)response.c_str(), response.length(), HAL_MAX_DELAY);
    }
}

void CLIManager::handleUARTData(uint8_t* data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        char ch = data[i];

        if (ch == '\r' || ch == '\n') {
            if (!inputBuffer.empty()) {
                CLICommand cmd(inputBuffer);
                xQueueSend(cliCommandQueue, &cmd, 0);
                inputBuffer.clear();
            }
        } else if (ch == '\b' || ch == 127) { // Backspace
            if (!inputBuffer.empty()) {
                inputBuffer.pop_back();
                sendResponse("\b \b");
            }
        } else if (inputBuffer.length() < MAX_INPUT_LENGTH) {
            inputBuffer += ch;
            sendResponse(std::string(1, ch)); // Echo character
        }
    }
}

void CLIManager::updateSystemStatus() {
    systemStatus.upTime = HAL_GetTick();
    systemStatus.freeHeap = xPortGetFreeHeapSize();
    systemStatus.activeSensors = sensorManager->getActiveSensorCount();
    systemStatus.state = SystemState::RUNNING;

    // Calculate CPU usage (simplified)
    static uint32_t lastIdleTicks = 0;
    uint32_t currentIdleTicks = xTaskGetIdleRunTimeCounter();
    uint32_t totalTicks = xTaskGetTickCount();

    if (totalTicks > 0) {
        systemStatus.cpuUsage = 100.0f * (1.0f - (float)(currentIdleTicks - lastIdleTicks) / (float)totalTicks);
    }
    lastIdleTicks = currentIdleTicks;
}

void CLIManager::update(const SensorData& data) {
    // Handle sensor data updates if needed
    // This is called when sensor data is available
}
