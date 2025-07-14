#include "system_monitor.hpp"

SystemMonitor::SystemMonitor(SensorManager* sensorMgr, CLIManager* cliMgr)
    : sensorManager(sensorMgr), cliManager(cliMgr), errorCount(0),
      lastHeartbeat(0), systemHealthy(true) {
    systemMutex = xSemaphoreCreateMutex();
    logger = Logger::getInstance();
}

SystemMonitor::~SystemMonitor() {
    stop();
    vSemaphoreDelete(systemMutex);
}

void SystemMonitor::init() {
    // Create watchdog timer (5 second timeout)
    watchdogTimer = xTimerCreate("WatchdogTimer", pdMS_TO_TICKS(5000),
                                pdTRUE, this, watchdogTimerCallback);

    // Create watchdog task
    xTaskCreate(watchdogTask, "WatchdogTask", WATCHDOG_TASK_STACK_SIZE,
                this, WATCHDOG_TASK_PRIORITY, &watchdogTaskHandle);

    logger->logInfo("System Monitor initialized", "SYS_MON");
}

void SystemMonitor::start() {
    xTimerStart(watchdogTimer, 0);
    heartbeat();
    logger->logInfo("System Monitor started", "SYS_MON");
}

void SystemMonitor::stop() {
    xTimerStop(watchdogTimer, 0);
    logger->logInfo("System Monitor stopped", "SYS_MON");
}

void SystemMonitor::heartbeat() {
    lastHeartbeat = HAL_GetTick();
}

void SystemMonitor::reportError(const std::string& error) {
    if (xSemaphoreTake(systemMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        errorCount++;
        systemHealthy = false;
        logger->logError("System error reported: " + error, "SYS_MON");
        xSemaphoreGive(systemMutex);
    }
}

void SystemMonitor::watchdogTask(void* parameter) {
    SystemMonitor* monitor = static_cast<SystemMonitor*>(parameter);

    while (true) {
        monitor->checkSystemHealth();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void SystemMonitor::watchdogTimerCallback(TimerHandle_t xTimer) {
    SystemMonitor* monitor = static_cast<SystemMonitor*>(pvTimerGetTimerID(xTimer));

    uint32_t currentTime = HAL_GetTick();
    if (currentTime - monitor->lastHeartbeat > 10000) { // 10 second timeout
        monitor->logger->logCritical("Watchdog timeout - system reset required", "WATCHDOG");
        monitor->handleSystemError();
    }
}

void SystemMonitor::checkSystemHealth() {
    if (xSemaphoreTake(systemMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        // Check heap memory
        uint32_t freeHeap = xPortGetFreeHeapSize();
        if (freeHeap < 1024) { // Less than 1KB free
            reportError("Low memory warning");
        }

        // Check task states
        if (eTaskGetState(sensorManager->getSensorTaskHandle()) == eDeleted) {
            reportError("Sensor task dead");
        }

        // Reset system health if no recent errors
        if (errorCount == 0) {
            systemHealthy = true;
        }

        xSemaphoreGive(systemMutex);
    }

    heartbeat();
}

void SystemMonitor::handleSystemError() {
    logger->logCritical("Handling system error", "SYS_MON");

    // Try to recover
    if (sensorManager) {
        sensorManager->resetAllSensors();
    }

    // If too many errors, reset system
    if (errorCount > 10) {
        resetSystem();
    }
}

void SystemMonitor::resetSystem() {
    logger->logCritical("System reset initiated", "SYS_MON");
    HAL_Delay(100);
    HAL_NVIC_SystemReset();
}

