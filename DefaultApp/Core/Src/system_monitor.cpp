#include "system_monitor.hpp"

SystemMonitor::SystemMonitor(SensorManager* sensorMgr, CLIManager* cliMgr)
    : sensorManager(sensorMgr), cliManager(cliMgr), errorCount(0),
      lastHeartbeat(0), systemHealthy(true) {
    systemMutex = xSemaphoreCreateMutex();
    logger = SystemLogger::getInstance();
}

SystemMonitor::~SystemMonitor() {
    stop();
    vSemaphoreDelete(systemMutex);
}

void StartWatchdogTask(void* argument){

}
void WatchdogTimerCallback(void const *argument);

void SystemMonitor::init() {

    // Create watchdog timer (5 second timeout)
    osTimerDef(watchdogTaskDef,  WatchdogTimerCallback);
    watchdogTimer = osTimerCreate(osThread(watchdogTaskDef), osTimerPeriodic, nullptr);

    // Create watchdog task
    osThreadDef(watchdogTaskDef, StartWatchdogTask, osPriorityNormal, 1, 512);
    watchdogTaskHandle = osThreadCreate(osThread(watchdogTaskDef), nullptr);



    logger->log(LogLevel::info, "System Monitor initialized", "SYS_MON");
}

void SystemMonitor::start() {
    osTimerStart(watchdogTimer, 0);
    heartbeat();
    logger->log(LogLevel::info,"System Monitor started", "SYS_MON");
}

void SystemMonitor::stop() {
    xTimerStop(watchdogTimer, 0);
    logger->log(LogLevel::info, "System Monitor stopped", "SYS_MON");
}

void SystemMonitor::heartbeat() {
    lastHeartbeat = HAL_GetTick();
}

void SystemMonitor::reportError(const std::string& error) {
    if (osSemaphoreWait(systemMutex, pdMS_TO_TICKS(1000)) == osOK) {
        errorCount++;
        systemHealthy = false;
        logger->log(LogLevel::error, "System error reported: " + error, "SYS_MON");
        osSemaphoreRelease(systemMutex);
    }
}

void SystemMonitor::watchdogTask(void* parameter) {
    SystemMonitor* monitor = static_cast<SystemMonitor*>(parameter);

    while (true) {
        monitor->checkSystemHealth();
        osDelay(pdMS_TO_TICKS(1000));
    }
}

void SystemMonitor::watchdogTimerCallback(TimerHandle_t xTimer) {
    SystemMonitor* monitor = static_cast<SystemMonitor*>(pvTimerGetTimerID(xTimer));

    uint32_t currentTime = HAL_GetTick();
    if (currentTime - monitor->lastHeartbeat > 10000) { // 10 second timeout
        monitor->logger->log(LogLevel::critical, "Watchdog timeout - system reset required", "WATCHDOG");
        monitor->handleSystemError();
    }
}

void SystemMonitor::checkSystemHealth() {
    if (osSemaphoreWait(systemMutex, pdMS_TO_TICKS(1000)) == osOK) {
        // Check heap memory
        uint32_t freeHeap = xPortGetFreeHeapSize();
        if (freeHeap < 1024) { // Less than 1KB free
            reportError("Low memory warning");
        }

//        // Check task states
//        if (eTaskGetState(sensorManager->getSensorTaskHandle()) == eDeleted) {
//            reportError("Sensor task dead");
//        }

        // Reset system health if no recent errors
        if (errorCount == 0) {
            systemHealthy = true;
        }

        osSemaphoreRelease(systemMutex);
    }

    heartbeat();
}

void SystemMonitor::handleSystemError() {
    logger->log(LogLevel::critical, "Handling system error", "SYS_MON");

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
    logger->log(LogLevel::critical, "System reset initiated", "SYS_MON");
    HAL_Delay(100);
    HAL_NVIC_SystemReset();
}

